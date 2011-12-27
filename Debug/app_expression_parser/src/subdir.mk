################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app_expression_parser/src/ExpressionParser.c 

OBJS += \
./app_expression_parser/src/ExpressionParser.o 

C_DEPS += \
./app_expression_parser/src/ExpressionParser.d 


# Each subdirectory must supply rules for building sources it contributes
app_expression_parser/src/%.o: ../app_expression_parser/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: C Compiler'
	xcc -O0 -g -Wall -c -std=gnu89 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $@ " -o $@ "$<"
	@echo 'Finished building: $<'
	@echo ' '


