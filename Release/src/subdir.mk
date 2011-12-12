################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/AOMDDFunction.cpp \
../src/Bucket.cpp \
../src/BucketTree.cpp \
../src/CompileBucket.cpp \
../src/CompileBucketTree.cpp \
../src/DDMiniBucket.cpp \
../src/DDMiniBucketTree.cpp \
../src/Graph.cpp \
../src/MetaNode.cpp \
../src/Model.cpp \
../src/NodeManager.cpp \
../src/PseudoTree.cpp \
../src/Scope.cpp \
../src/TableFunction.cpp \
../src/parsers.cpp \
../src/utils.cpp 

OBJS += \
./src/AOMDDFunction.o \
./src/Bucket.o \
./src/BucketTree.o \
./src/CompileBucket.o \
./src/CompileBucketTree.o \
./src/DDMiniBucket.o \
./src/DDMiniBucketTree.o \
./src/Graph.o \
./src/MetaNode.o \
./src/Model.o \
./src/NodeManager.o \
./src/PseudoTree.o \
./src/Scope.o \
./src/TableFunction.o \
./src/parsers.o \
./src/utils.o 

CPP_DEPS += \
./src/AOMDDFunction.d \
./src/Bucket.d \
./src/BucketTree.d \
./src/CompileBucket.d \
./src/CompileBucketTree.d \
./src/DDMiniBucket.d \
./src/DDMiniBucketTree.d \
./src/Graph.d \
./src/MetaNode.d \
./src/Model.d \
./src/NodeManager.d \
./src/PseudoTree.d \
./src/Scope.d \
./src/TableFunction.d \
./src/parsers.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/Users/willmlam/dev/researchProjects/aomdd/include" -I/opt/local/include -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


