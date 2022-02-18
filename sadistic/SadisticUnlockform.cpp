#include "SadisticUnlockform.h"

struct Spinner  : public juce::Component, private juce::Timer {
    Spinner()                       { startTimer (1000 / 50); }
    void timerCallback() override   { repaint(); }
    void paint (Graphics& g) override {
        getLookAndFeel().drawSpinningWaitAnimation (g, Colours::darkgrey, 0, 0, getWidth(), getHeight());
    }
};
    
struct sadistic::SadisticUnlockForm::OverlayComp  : public juce::Component, private juce::Thread, private Timer {
    OverlayComp (SadisticUnlockForm& f) : Thread (String()), form (f) {
        result.succeeded = false;
        email = form.emailBox.getText();
        password = form.passwordBox.getText();
        addAndMakeVisible (spinner);
        startThread (4);
    }
    
    ~OverlayComp() override { stopThread (10000); }
    
    void paint (Graphics& g) override {
        g.fillAll (Colours::black.withAlpha (0.7f));
        g.setColour (Colours::lightslategrey.brighter());
        g.setFont (getSadisticFont());
        g.drawFittedText (translate("Contacting XYZ...").replace ("XYZ", form.status.getWebsiteName()),
                          getLocalBounds().reduced (20, 0).removeFromTop (proportionOfHeight (0.6f)),
                          Justification::centred, 5);
    }
    
    void resized() override {
        const int spinnerSize = 40;
        spinner.setBounds ((getWidth() - spinnerSize) / 2, proportionOfHeight (0.6f), spinnerSize, spinnerSize);
    }
    
    void run() override {
        result = form.status.attemptWebserverUnlock (email, password);
        startTimer (100);
    }
    
    void timerCallback() override {
        spinner.setVisible (false);
        stopTimer();
        
        if (result.errorMessage.isNotEmpty()) {
            juce::AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Registration Failed!", result.errorMessage);
        }
        else if (result.informativeMessage.isNotEmpty()) {
            juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Registration Complete!", result.informativeMessage);
        }
        
        // (local copies because we're about to delete this)
        const bool worked = result.succeeded;
        if (worked) form.status.save();
        SadisticUnlockForm& f = form;
        delete this;
        if (worked) {
            if(!f.isPlugin()) {
                f.saveButton.setEnabled(true);
                f.statusLabel.setText("Status: Key Acquired, please Save", sendNotificationSync);
            }
        }
    }

    SadisticUnlockForm& form;
    Spinner spinner;
    OnlineUnlockStatus::UnlockResult result;
    String email, password;

    JUCE_LEAK_DETECTOR (SadisticUnlockForm::OverlayComp)
};

String sadistic::SadisticUnlockForm::getMessage() {
    String defaultInstructions { "Thank you for your purchase! Note: This app requires an internet connection and a license request file.  After installing a plug-in on the offline machine, a license request file can be generated from within the plug-in.  Press the 'Load' button to load a license request file (.xml).  Then enter your Sadistic Audio account email and password and press 'Register'.  If successful, press 'Save' to save your license key." };
    
    String pluginInstructions { "Please enter your email address and password associated with your shop.sadisticaudio.com account.  If you require offline authorization, you can save a request file and/or load a license key below.  A seperate Sadistic Authorizer app can be obtained by signing in to your Sadistic Audio Shop account and heading to the Downloads section.  Thank you for your purchase!" };
    
    return isPlugin() ? pluginInstructions : defaultInstructions;
}

sadistic::SadisticUnlockForm::SadisticUnlockForm (SadisticMarketplaceStatus& s) : status (s), message (String(), getMessage()) {
    
    productList.onChange = [this] { status.product = status.getProductID(productList.getText()); };
    
    if(isPlugin()) {
        
        saveCallback = [&,this] (const FileChooser& chooser) {
            if (!chooser.getResults().isEmpty()) {
                auto requestFile { chooser.getResult() };
                if (!requestFile.existsAsFile()) requestFile.create();
                if(auto xmlElement = std::make_unique<XmlElement>("root")) {
                    xmlElement->setAttribute ("product", status.getProductID());
                    xmlElement->setAttribute ("os", juce::SystemStats::getOperatingSystemName());
                    xmlElement->setAttribute ("mach", status.getLocalMachineIDs()[0]);
                    if(xmlElement->writeTo(requestFile)) {
                        statusLabel.setText("Status: Request Saved", sendNotificationSync);
                        juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Saved...", "Transfer this file to a computer with internet connectivity and use it to generate a license key file using the Sadistic Authorizer application, which can be obtained in the downloads section of the web store, after you have logged in.");
                    }
                    else juce::AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Error...", "Unable to save Key Request File...");
                }
            }
        };

        saveFile = [&,this](){
            fc = std::make_unique<FileChooser>("Save License Request File", File("~/Desktop/licenseKeyRequest.xml"));
            
            fc->launchAsync (FileBrowserComponent::saveMode |
                             FileBrowserComponent::canSelectFiles |
                             FileBrowserComponent::warnAboutOverwriting |
                             FileBrowserComponent::doNotClearFileNameOnRootChange,
                             saveCallback);
        };
        
        loadCallback = [&,this] (const FileChooser& chooser) {
            if (!chooser.getResults().isEmpty()) {
                if(auto keyXml = parseXML(chooser.getResult())) {
                    auto unlockResult = status.handleOfflineXml(*keyXml);
                    const bool worked = unlockResult.succeeded;
                    if (worked) {
                        status.save();
                        if (unlockResult.informativeMessage.isNotEmpty())
                            juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Registration Complete!", unlockResult.informativeMessage);
                    }
                    else statusLabel.setText("Status: Invalid / Empty Key File", sendNotificationSync);
                }
            }
        };
        
        loadFile = [&,this](){
            fc = std::make_unique<FileChooser>("Load License Key File (.xml)", File::getSpecialLocation(File::userDesktopDirectory), "*.xml");
            
            fc->launchAsync (FileBrowserComponent::openMode |
                             FileBrowserComponent::canSelectFiles,
                             loadCallback);
        };
        productList.addItem(status.getProductName(status.product), 1);
        productList.setSelectedId(1);
    }
    else {
        saveButton.setEnabled(false);
        registerButton.setEnabled(false);
        
        loadCallback = [&,this] (const FileChooser& chooser) {
            if (!chooser.getResults().isEmpty()) {
                if(auto xml = parseXML(chooser.getResult())) {
                    status.product = xml->getStringAttribute("product");
                    if(status.product.isNotEmpty()) productList.setText(status.getProductName(status.product));
                    status.os = xml->getStringAttribute("os");
                    status.mach = xml->getStringAttribute("mach");
                    if(status.product.isNotEmpty() && status.os.isNotEmpty() && status.mach.isNotEmpty()) {
                        registerButton.setEnabled(true);
                        statusLabel.setText("Status: Request File Loaded", sendNotificationSync);
                    }
                    else juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Warning", "Xml Request file seems to have invalid content.");
                }
                else juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Error", "Unable to Parse Xml file.");
            }
        };
        
        loadFile = [&,this](){
            fc = std::make_unique<FileChooser>("Load License Key Request File (.xml)", File::getSpecialLocation(File::userDesktopDirectory), "*.xml");
            
            fc->launchAsync (FileBrowserComponent::openMode |
                             FileBrowserComponent::canSelectFiles,
                             loadCallback);
        };
        
        saveCallback = [&,this] (const FileChooser& chooser) {
            if (!chooser.getResults().isEmpty()) {
                if(status.storedKey) {
                    if(status.storedKey->writeTo(chooser.getResult())) {
                        statusLabel.setText("Status: Key Saved", sendNotificationSync);
                        juce::AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Saved", "Key File Saved...");
                    }
                    else juce::AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Error", "Unable To Save Key...");
                }
                else juce::AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Error", "No Key To Save...");
            }
        };
        
        saveFile = [&,this](){
            fc = std::make_unique<FileChooser>("Save License Key File",  File("~/Desktop/licenseKey.xml"));

            fc->launchAsync (FileBrowserComponent::saveMode |
                             FileBrowserComponent::canSelectFiles |
                             FileBrowserComponent::warnAboutOverwriting |
                             FileBrowserComponent::doNotClearFileNameOnRootChange,
                             saveCallback);
        };
        
        for (int i = 1; i <= (int)(sizeof(products)/sizeof(Product)); i++) productList.addItem(String(products[i].name), i);
    }

    loadButton.onClick = [&,this](){ loadFile(); };
    saveButton.onClick = [&,this](){ saveFile(); };
    registerButton.onClick = [&,this](){ attemptRegistration(); };

    message.setJustificationType (Justification::centred);
    statusLabel.setJustificationType (Justification::horizontallyJustified);
    statusLabel.setText("Status: ", sendNotificationSync);

    logo = Drawable::createFromImageData (Data::saLogo_svg, Data::saLogo_svgSize);
    
    addChildComponent (logo.get());
    addAndMakeVisible (statusLabel);
    addAndMakeVisible (message);
    addAndMakeVisible (emailBox);
    addAndMakeVisible (passwordBox);
    addAndMakeVisible (productList);

    addAndMakeVisible (registerButton);
    addAndMakeVisible (loadButton);
    addAndMakeVisible (saveButton);

    Colour labelCol (findColour (TextEditor::backgroundColourId).contrasting (0.5f));
    emailBox.setTextToShowWhenEmpty ("Email Address", labelCol);
    passwordBox.setTextToShowWhenEmpty ("Password", labelCol);
    productList.setTextWhenNothingSelected("Sadistic Product Name");
}

sadistic::SadisticUnlockForm::~SadisticUnlockForm() { unlockingOverlay.deleteAndZero(); }

void sadistic::SadisticUnlockForm::paint (Graphics& g) {
    g.fillAll(Colours::black);
    logo->drawWithin(g, logo->getBounds().toFloat(), 64, 1.f);
}

void sadistic::SadisticUnlockForm::resized() {
//        jassert (JUCEApplicationBase::isStandaloneApp() || findParentComponentOfClass<DialogWindow>() == nullptr);
    logo->centreWithSize((int)((float) getHeight() * 0.93f), (int)((float) getHeight() / 0.93f));
    const int buttonHeight = 22;
    auto r = getLocalBounds().reduced (10, 20);
    auto buttonArea = r.removeFromBottom (buttonHeight);
    registerButton.changeWidthToFitText (buttonHeight);
    loadButton.changeWidthToFitText (buttonHeight);
    saveButton.changeWidthToFitText (buttonHeight);
    const int gap = 20;
    buttonArea = buttonArea.withSizeKeepingCentre (registerButton.getWidth() + gap + loadButton.getWidth() + gap + saveButton.getWidth(), buttonHeight);
    registerButton.setBounds (buttonArea.removeFromLeft (registerButton.getWidth()));
    buttonArea.removeFromLeft (gap);
    loadButton.setBounds (buttonArea.removeFromLeft (loadButton.getWidth()));
    buttonArea.removeFromLeft (gap);
    saveButton.setBounds (buttonArea.removeFromLeft (saveButton.getWidth()));
    statusLabel.setBounds(saveButton.getBounds().withWidth(getBoundsInParent().getRight() - saveButton.getRight()).withX(saveButton.getRight()));
    
    r.removeFromBottom (20);
    // (force use of a default system font to make sure it has the password blob character)
    Font font(Font::getDefaultTypefaceForFont(Font(Font::getDefaultSansSerifFontName(), Font::getDefaultStyle(), 5.0f)));
    
    productList.setBounds(r.removeFromBottom(buttonHeight));
    
    r.removeFromBottom(20);
    
    const int boxHeight = 24;
    passwordBox.setBounds (r.removeFromBottom (boxHeight));
    passwordBox.setInputRestrictions (64);
    passwordBox.setFont (font);
    
    r.removeFromBottom (20);
    emailBox.setBounds (r.removeFromBottom (boxHeight));
    emailBox.setInputRestrictions (512);
    emailBox.setFont (font);
    
    r.removeFromBottom (20);
    
    message.setBounds (r);
    
    if (unlockingOverlay != nullptr)
        unlockingOverlay->setBounds (getLocalBounds());
}

void sadistic::SadisticUnlockForm::showBubbleMessage (const String& text, Component& target) {
    bubble.reset (new BubbleMessageComponent (500));
    addChildComponent (bubble.get());
    
    AttributedString attString;
    attString.append (text, Font (16.0f));
    
    bubble->showAt (getLocalArea (&target, target.getLocalBounds()),
                    attString, 500,  // numMillisecondsBeforeRemoving
                    true,  // removeWhenMouseClicked
                    false); // deleteSelfAfterUse
}

void sadistic::SadisticUnlockForm::attemptRegistration() {
    
    if (!unlockingOverlay) {
        if (emailBox.getText().trim().length() < 3) {
            showBubbleMessage ("Please enter a valid email address!", emailBox);
            return;
        }
        if (passwordBox.getText().trim().length() < 3) {
            showBubbleMessage ("Please enter a valid password!", passwordBox);
            return;
        }
        status.setUserEmail (emailBox.getText());
        addAndMakeVisible (unlockingOverlay = new OverlayComp (*this));
        resized();
        unlockingOverlay->enterModalState();
    }
}
