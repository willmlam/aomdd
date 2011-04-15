################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Bucket.cpp \
../src/BucketTree.cpp \
../src/Model.cpp \
../src/Scope.cpp \
../src/TableFunction.cpp \
../src/parsers.cpp \
../src/utils.cpp 

OBJS += \
./src/Bucket.o \
./src/BucketTree.o \
./src/Model.o \
./src/Scope.o \
./src/TableFunction.o \
./src/parsers.o \
./src/utils.o 

CPP_DEPS += \
./src/Bucket.d \
./src/BucketTree.d \
./src/Model.d \
./src/Scope.d \
./src/TableFunction.d \
./src/parsers.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/willmlam/dev/graphmod/aomdd/include" -O2 -g -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


