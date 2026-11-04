################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utils/Src/Callback.c \
../Utils/Src/Extra.c \
../Utils/Src/Global_Var.c \
../Utils/Src/Justfloat.c \
../Utils/Src/PID.c \
../Utils/Src/Sample.c 

OBJS += \
./Utils/Src/Callback.o \
./Utils/Src/Extra.o \
./Utils/Src/Global_Var.o \
./Utils/Src/Justfloat.o \
./Utils/Src/PID.o \
./Utils/Src/Sample.o 

C_DEPS += \
./Utils/Src/Callback.d \
./Utils/Src/Extra.d \
./Utils/Src/Global_Var.d \
./Utils/Src/Justfloat.d \
./Utils/Src/PID.d \
./Utils/Src/Sample.d 


# Each subdirectory must supply rules for building sources it contributes
Utils/Src/%.o Utils/Src/%.su Utils/Src/%.cyclo: ../Utils/Src/%.c Utils/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I"D:/STM32CubeIDE_WorkSpace/G4/FSBB_Self/Utils/Inc" -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Utils-2f-Src

clean-Utils-2f-Src:
	-$(RM) ./Utils/Src/Callback.cyclo ./Utils/Src/Callback.d ./Utils/Src/Callback.o ./Utils/Src/Callback.su ./Utils/Src/Extra.cyclo ./Utils/Src/Extra.d ./Utils/Src/Extra.o ./Utils/Src/Extra.su ./Utils/Src/Global_Var.cyclo ./Utils/Src/Global_Var.d ./Utils/Src/Global_Var.o ./Utils/Src/Global_Var.su ./Utils/Src/Justfloat.cyclo ./Utils/Src/Justfloat.d ./Utils/Src/Justfloat.o ./Utils/Src/Justfloat.su ./Utils/Src/PID.cyclo ./Utils/Src/PID.d ./Utils/Src/PID.o ./Utils/Src/PID.su ./Utils/Src/Sample.cyclo ./Utils/Src/Sample.d ./Utils/Src/Sample.o ./Utils/Src/Sample.su

.PHONY: clean-Utils-2f-Src

