################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Model.cpp \
../src/Scope.cpp \
../src/TableFunction.cpp \
../src/parsers.cpp \
../src/utils.cpp 

OBJS += \
./src/Model.o \
./src/Scope.o \
./src/TableFunction.o \
./src/parsers.o \
./src/utils.o 

CPP_DEPS += \
./src/Model.d \
./src/Scope.d \
./src/TableFunction.d \
./src/parsers.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/Users/willmlam/dev/workspaces/graphmod/aomdd/include" -I/opt/local/include -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


