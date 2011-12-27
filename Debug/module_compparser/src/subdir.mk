################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../module_compparser/src/CompParser.c \
../module_compparser/src/EbnfGrammarCompiler.c 

OBJS += \
./module_compparser/src/CompParser.o \
./module_compparser/src/EbnfGrammarCompiler.o 

C_DEPS += \
./module_compparser/src/CompParser.d \
./module_compparser/src/EbnfGrammarCompiler.d 


# Each subdirectory must supply rules for building sources it contributes
module_compparser/src/%.o: ../module_compparser/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: C Compiler'
	xcc -O0 -g -Wall -c -std=gnu89 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $@ " -o $@ "$<"
	@echo 'Finished building: $<'
	@echo ' '


