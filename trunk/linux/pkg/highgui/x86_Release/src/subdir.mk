################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/bitstrm.cpp \
../src/cvcap.cpp \
../src/cvcap_cmu.cpp \
../src/cvcap_dc1394.cpp \
../src/cvcap_images.cpp \
../src/dummy.cpp \
../src/grfmt_base.cpp \
../src/grfmt_exr.cpp \
../src/grfmt_imageio.cpp \
../src/grfmt_jpeg.cpp \
../src/grfmt_jpeg2000.cpp \
../src/grfmt_png.cpp \
../src/grfmt_pxm.cpp \
../src/grfmt_sunras.cpp \
../src/grfmt_tiff.cpp \
../src/image.cpp \
../src/loadsave.cpp \
../src/precomp.cpp \
../src/utils.cpp \
../src/window.cpp \
../src/window_gtk.cpp \
../src/window_w32.cpp 

OBJS += \
./src/bitstrm.o \
./src/cvcap.o \
./src/cvcap_cmu.o \
./src/cvcap_dc1394.o \
./src/cvcap_images.o \
./src/dummy.o \
./src/grfmt_base.o \
./src/grfmt_exr.o \
./src/grfmt_imageio.o \
./src/grfmt_jpeg.o \
./src/grfmt_jpeg2000.o \
./src/grfmt_png.o \
./src/grfmt_pxm.o \
./src/grfmt_sunras.o \
./src/grfmt_tiff.o \
./src/image.o \
./src/loadsave.o \
./src/precomp.o \
./src/utils.o \
./src/window.o \
./src/window_gtk.o \
./src/window_w32.o 

CPP_DEPS += \
./src/bitstrm.d \
./src/cvcap.d \
./src/cvcap_cmu.d \
./src/cvcap_dc1394.d \
./src/cvcap_images.d \
./src/dummy.d \
./src/grfmt_base.d \
./src/grfmt_exr.d \
./src/grfmt_imageio.d \
./src/grfmt_jpeg.d \
./src/grfmt_jpeg2000.d \
./src/grfmt_png.d \
./src/grfmt_pxm.d \
./src/grfmt_sunras.d \
./src/grfmt_tiff.d \
./src/image.d \
./src/loadsave.d \
./src/precomp.d \
./src/utils.d \
./src/window.d \
./src/window_gtk.d \
./src/window_w32.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/mamigo/ws_nanoNXT/linux/pkg/cv/include" -I"/home/mamigo/ws_nanoNXT/linux/pkg/cxcore/include" -I"/home/mamigo/ws_nanoNXT/linux/pkg/highgui/include" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


