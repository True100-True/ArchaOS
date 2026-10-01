curl -O https://ftp.gnu.org/gnu/gcc/gcc-15.3.0/gcc-15.3.0.tar.xz
tar xf gcc-15.3.0.tar.xz

# be sure binutils is installed
x86_64-elf-as --version
if [ $? -ne 0 ]; then
  exit 1
fi

cd gcc-15.3.0
./contrib/download_prerequisites
cd ..

mkdir build-gcc
cd build-gcc

../gcc-15.3.0/configure \
    --target=x86_64-elf \
    --prefix="$HOME/cross" \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers \
    --disable-multilib

set CONFIG_SUCCEED_ERROR=$?

if [ CONFIG_SUCCEED_ERROR -ne 0 ]; then
  echo "Config did not succeed ($(CONFIG_SUCCEED_ERROR))"
  exit 1
fi;

make all-gcc -j"$(nproc)"
make all-target-libgcc -j"$(nproc)" 

grep -n '^install:' gcc/Makefile
grep -n '^install-gcc' Makefile
ls -la gcc/Makefile
ls -la x86_64-elf/

make install-gcc
make install-target-libgcc -j"$(nproc)"
echo "exit code = $?"

x86_64-elf-gcc --version
x86_64-elf-g++ --version

###########################################################

curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.45.tar.xz
tar xf binutils-2.45.tar.xz

mkdir build-binutils
cd build-binutils

mkdir $HOME/cross/

../binutils-2.45/configure \
    --target=x86_64-elf \
    --prefix=$HOME/cross/
    --disable-werror \
    --disable-nls
  
make -j$(nproc)
make install

x86_64-elf-as --version
x86_64-elf-ld --version
