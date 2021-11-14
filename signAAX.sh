#! /bin/bash

test -f ~/CODE/sadistic/Deviant/cmake-build/Deviant_artefacts/Release/AAX/Deviant.aaxplugin/Contents/MacOS/Deviant &&
wraptool sign --verbose --account likemoons --signid "Developer ID Application: Benjamin Kelley (NXW2SG7JJ4)" --extrasigningoptions "--timestamp" --dsigharden --wcguid F9023250-B36D-11EB-BADE-005056920FF7 --in ~/CODE/sadistic/Deviant/cmake-build/Deviant_artefacts/Release/AAX/Deviant.aaxplugin --out ~/CODE/sadistic/Deviant/cmake-build/Deviant_artefacts/Release/AAX/Signed/Deviant.aaxplugin
