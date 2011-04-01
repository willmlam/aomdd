################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Scope.cpp \
../src/parsers.cpp \
../src/utils.cpp 

OBJS += \
./src/Scope.o \
./src/parsers.o \
./src/utils.o 

CPP_DEPS += \
./src/Scope.d \
./src/parsers.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/Users/willmlam/dev/workspaces/graphmod/aomdd/include" -I/opt/local/include -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


