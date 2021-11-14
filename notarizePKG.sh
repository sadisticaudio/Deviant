#!/bin/bash

USERNAME="frye@sadisticaudio.com"
PASSWORD="fslz-cvuu-svdo-wqyk"
PKG="$HOME/CODE/sadistic/Deviant/Installer/build/sadisticaudioDeviant.pkg"

rm ~/Library/Caches/com.apple.amp.itmstransporter/UploadTokens/*.token

echo "Submitting to Apple..."
xcrun altool --notarize-app -f $PKG --primary-bundle-id "com.sadisticaudio.Deviant" --username $USERNAME --password $PASSWORD &> notarization.result

ASSET_UUID=`grep RequestUUID notarization.result | cut -d" " -f 3`

echo -n "Checking result of notarisation.."

while true; do
	echo -n '.'
	if [[ `xcrun altool --notarization-info $ASSET_UUID --username $USERNAME --password $PASSWORD 2>&1  >/dev/null | grep -c "Package Approved"` == "1" ]]; then
		break
	fi
	sleep 30
done

echo
echo "Stapling package..."

xcrun stapler staple "$PKG"

current_time=$(date "+%Y.%m.%d-%H.%M.%S")
mv notarization.result Other/notarization.$current_time.result
