# Viblio Face Detector Linux Build Instructions

1. Check out the viblio/faces repository:

  ```
  git clone git@github.com:viblio/faces
  cd faces
  ```
 
2. If you haven't already, remove the Ubuntu packaged Boost (1.46 or
  1.48 on Ubuntu 12.04), and replace it with Boost 1.55.  You can find
  your boost version by looking in ```/usr/local/include/boost/version.hpp```.

  ```
  wget -O boost_1_55_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz/download
  tar xzvf boost_1_55_0.tar.gz
  cd boost_1_55_0/
  ./bootstrap.sh --prefix=/usr/local
  sudo ./b2 link=static,shared --with=all install
  sudo apt-get -y remove `dpkg --get-selections | grep boost | grep -v deinstall | awk '{ print $1; }'`
  ```

3. Establish prerequisites:

  ```
  sudo make install_linux_deps
  ```

  This step will take a long time the first time you run it, as it
  installs the Neurotechnology VeriLook SDK which is a ~700 MB download
  which is then unpacked into a ~2 GB directory structure.

  The first time you run this command it will produce a harmless error
  on attempting to rename a directory, you should ignore an error like
  this:

  ```
  ...
  mv /opt/Neurotec_SDK /opt/Neurotec_SDK.prev
  mv: cannot stat `/opt/Neurotec_SDK': No such file or directory
  make[1]: [install] Error 1 (ignored)
  mv /opt/Neurotec_SDK.next /opt/Neurotec_SDK
  ( cd /opt ; chown -R www-data:www-data Neurotec_SDK )
  make[1]: Leaving directory `/tmp/jUAUL_dMMx'
  make: *** [install_linux_deps] Error 1
  ```

  NOTES: 

  * This step assumes you have already followed the directions on
[https://github.com/viblio/video_processor/wiki/Building-a-new-development-machine](setting up your development environment). Those directions install a myriad of required libraries and tools.
  * After install_linux_deps our license file, which is not part of the original SDK, can be found at:
    * /opt/Neurotec_SDK/Bin/Linux_x86_64/Activation/pgd.conf
    * /opt/Neurotec_SDK/Bin/Linux_x86_64/Activation/license_file.lic
      * This is our original license file with 6 FaceClients
    * /opt/Neurotec_SDK/Bin/Linux_x86_64/Activation/license_file2.lic
      * This is our update license file with 5 FaceClients
  * The original SDK (without license files) can be obtained from:
    * Address: http://www.neurotechnology.com/cgi-bin/support.cgi#pu
    * Password: fz_VGzwMiCr(IX1L

4. Build the code:

  ```
  make clean
  make
  ```

5. To deploy the build on a local development machine:

  ```
  mkdir foo
  cp package.tar.gz foo
  cd foo
  tar xvvpfz package.tar.gz
  sudo make install
  ```

6. To push the code to staging:

  ```
  make clean
  make
  make bump
  ```
