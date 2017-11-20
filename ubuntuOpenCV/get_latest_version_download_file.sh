# Dan Walkes
# 2014-01-29
# Find the latest version and download file link from the OpenCV sourceforge page

version=3.3.1
downloadfilelist="opencv-$version.tar.gz opencv-$version.zip"
downloadfile=
for file in $downloadfilelist;
do
        wget --spider http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/$version/$file/download
        if [ $? -eq 0 ]; then
                downloadfile=$file
        fi
done
if [ -z "$downloadfile" ]; then
        echo "Could not find download file on sourceforge page.  Please find the download file for version $version at"
        echo "http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/$version/ and update this script"
        exit  1
fi
