#pragma once
#include "../../Source/sadistic.h"

namespace sadistic {
    
    struct Product {
        int idx; const char* id; const char* name;
    };
    static constexpr Product products[] {
        { 0, "faKe", "Fake" },
        { 1, "hieF", "Thief" },
        { 2, "iAnt", "Deviant" },
        { 3, "verT", "Pervert" }
    };
    
    struct SadisticMarketplaceStatus : public juce::OnlineUnlockStatus {

        static inline String getProductID(int idx) { return String(products[idx].id); }
        static inline String getProductName(int idx) { return String(products[idx].name); }
        static inline int getProductIndexFromName(String name, int i = 0) {
            for(; i < (int)(sizeof(products)/sizeof(Product)); ++i)
                if(name == String(products[i].name))
                    return i;
            return 0;
        }
        static inline int getProductIndexFromID(String pID) {
            for(int i { 0 }; i < (int)(sizeof(products)/sizeof(Product)); ++i)
                if(pID == String(products[i].id))
                    return i;
            return 0;
        }
        static inline String getProductID(String name) { return getProductID(getProductIndexFromName(name)); }
        static inline String getProductName(String pID) { return getProductName(getProductIndexFromID(pID)); }
        
        SadisticMarketplaceStatus(String productCode = String()) : isPlugin(productCode.isNotEmpty() ? true : false), product(isPlugin ? productCode : String()), name(isPlugin ? getProductName(productCode) : String()) {}
        void paint(Graphics& g) { g.fillAll(Colours::black.withAlpha(0.5f)); }
        juce::String getProductID() override { return product.isNotEmpty() ? product : String(); }
        bool doesProductIDMatch (const juce::String& returnedIDFromServer) override { return getProductID() == returnedIDFromServer; }
        juce::RSAKey getPublicKey() override { return juce::RSAKey ("5,ac317e992ce3c4833168fb8037f9937fbed34d77d3982942ed60682db20724b9"); }
        
        String getPersistenceFilePath() {
            File homePath { File::getSpecialLocation(File::userHomeDirectory) };
            std::vector<File> files {
                { { File(File::getSpecialLocation(File::userApplicationDataDirectory).getFullPathName() + "/Application Support/Sadistic Audio/persistentData.xml") } },
                { { File(File::getSpecialLocation(File::userMusicDirectory).getFullPathName() + "/.sadisticaudio/persistentData.xml") } }
            };
            for(size_t i = 0; i < files.size(); ++i) {
                auto result = files[i].create();
                if(result.wasOk())
                    return files[i].getFullPathName();
            }
            return {};
        }
        
        File getPersistenceFile() { return File(getPersistenceFilePath()); }
        
        void saveState (const juce::String& s) override {
            if(isPlugin) {
                auto rootElement = parseXML(getPersistenceFile());
                if(!rootElement) rootElement = std::make_unique<XmlElement>(translate("root"));
                XmlElement* element;
                if(!(element = rootElement->getChildByName(product))) element = rootElement->createNewChildElement(product);
                element->setAttribute(translate("marketplacestatus"), s);
                rootElement->writeTo(getPersistenceFile(), XmlElement::TextFormat());
            }
        }
        juce::String getState() override {
            if(auto rootElement = parseXML(getPersistenceFile())) {
                if(auto element = rootElement->getChildByName(product))
                    return String(element->getStringAttribute(translate("marketplacestatus")));
            } return {};
        }
        juce::String getWebsiteName() override { return "auth.sadisticaudio.com"; }
        juce::URL getServerAuthenticationURL() override { return juce::URL ("https://auth.sadisticaudio.com/auth.php"); }
        
        OnlineUnlockStatus::UnlockResult handleOfflineXml (XmlElement xml) {
            UnlockResult r;
            if (auto keyNode = xml.getChildByName ("KEY")) {
                const String keyText (keyNode->getAllSubText().trim());
                r.succeeded = keyText.length() > 10 && applyKeyFile (keyText);
            }
            else r.succeeded = false;
            if (xml.hasTagName ("MESSAGE")) r.informativeMessage = xml.getStringAttribute ("message").trim();
            if (xml.hasTagName ("ERROR")) r.errorMessage = xml.getStringAttribute ("error").trim();
            if (xml.getStringAttribute ("url").isNotEmpty()) r.urlToLaunch = xml.getStringAttribute ("url").trim();
            if (r.errorMessage.isEmpty() && r.informativeMessage.isEmpty() && r.urlToLaunch.isEmpty() && ! r.succeeded)
                r.errorMessage = getMessageForUnexpectedReply();
            return r;
        }
        
        juce::String readReplyFromWebserver (const juce::String& email, const juce::String& password) override {

            auto p { product.isNotEmpty() ? product : getProductID() };
            auto o { os.isNotEmpty() ? os : juce::SystemStats::getOperatingSystemName() };
            auto m { mach.isNotEmpty() ? mach : getLocalMachineIDs()[0] };
            juce::URL url (getServerAuthenticationURL()
                           .withParameter ("product", p)
                           .withParameter ("email", email)
                           .withParameter ("pw", password)
                           .withParameter ("os", o)
                           .withParameter ("mach", m));
            
            DBG ("Trying to unlock via URL: " << url.toString (true));
            {// <---                                                        weird, this must just limit the scope here
                juce::ScopedLock lock (streamCreationLock);//<---           weird, this must just limit the scope here
                stream.reset (new juce::WebInputStream (url, true));//<---  weird, this must just limit the scope here
            }// <---                                                        weird, this must just limit the scope here
            
            if (stream->connect (nullptr)) {
                auto* thread = juce::Thread::getCurrentThread();
                if (thread->threadShouldExit() || stream->isError()) return {};
                auto contentLength = stream->getTotalLength();
                auto downloaded    = 0;
                const size_t bufferSize = 0x8000;
                juce::HeapBlock<char> buffer (bufferSize);
                while (! (stream->isExhausted() || stream->isError() || thread->threadShouldExit())) {
                    auto max = juce::jmin ((int) bufferSize, contentLength < 0 ? std::numeric_limits<int>::max()
                                           : static_cast<int> (contentLength - downloaded));
                    auto actualBytesRead = stream->read (buffer.get() + downloaded, max - downloaded);
                    if (actualBytesRead < 0 || thread->threadShouldExit() || stream->isError()) break;
                    downloaded += actualBytesRead;
                    if (downloaded == contentLength) break;
                }
                if (thread->threadShouldExit() || stream->isError() || (contentLength > 0 && downloaded < contentLength)) return {};

                String temp { juce::CharPointer_UTF8 (buffer.get()) };
                if(isPlugin == false) storedKey = parseXML(temp);
                return temp;
            }
            return {};
        }

        const bool isPlugin;
        std::unique_ptr<XmlElement> storedKey;
        String product, name, mach, os;
        juce::CriticalSection streamCreationLock;
        std::unique_ptr<juce::WebInputStream> stream;
    };

    struct SadisticUnlockForm  : public juce::Component {
        SadisticUnlockForm (SadisticMarketplaceStatus&);
        ~SadisticUnlockForm() override;
        SadisticMarketplaceStatus& status;
        String getMessage();
        void paint (Graphics&) override;
        void resized() override;
        void dismiss() { }
        bool isPlugin() { return status.isPlugin; }
        Label message, statusLabel { String(), "Status: " };
        TextEditor emailBox, passwordBox { String(), juce_wchar(0x25cf) };
        ComboBox productList;
        TextButton registerButton { "Register" }, loadButton { "Load" }, saveButton { "Save" };
        struct OverlayComp;
        friend struct OverlayComp;

        std::unique_ptr<Drawable> logo;
        std::unique_ptr<FileChooser> fc;
        std::function<void()> loadFile, saveFile;
        std::function<void (const FileChooser&)> loadCallback, saveCallback;
        std::unique_ptr<BubbleMessageComponent> bubble;
        Component::SafePointer<Component> unlockingOverlay;
        void attemptRegistration();
        void showBubbleMessage (const String&, Component&);
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SadisticUnlockForm)
    };
} // namespace sadistic
