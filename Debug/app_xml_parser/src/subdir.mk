################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app_xml_parser/src/XmlParser.c 

OBJS += \
./app_xml_parser/src/XmlParser.o 

C_DEPS += \
./app_xml_parser/src/XmlParser.d 


# Each subdirectory must supply rules for building sources it contributes
app_xml_parser/src/%.o: ../app_xml_parser/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: C Compiler'
	xcc -O0 -g -Wall -c -std=gnu89 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $@ " -o $@ "$<"
	@echo 'Finished building: $<'
	@echo ' '


