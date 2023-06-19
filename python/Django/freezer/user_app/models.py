from django.db import models
# 创建  系统用户信息表
class UserTable(models.Model):
    # 默认自动创建主键ID
    user_code = models.CharField(max_length=20)  # 账号 默认不为空
    user_password = models.CharField(max_length=20)  # 密码
    user_name = models.CharField(max_length=10)  # 用户的名字昵称
    user_mail = models.CharField(max_length=100, null=True)  # 邮箱可以为空
    user_phone = models.CharField(max_length=20, null=True)  # 电话号码
    user_type = models.CharField(max_length=1)  # 部门  0用户  默认注册  1管理员（只有一个）
    remark = models.CharField(max_length=500, null=True)  # 备注
    op_time = models.DateTimeField(auto_now_add=True)  # 创建时 更新为当前时间
#传感器数据表
class SensorDataTable(models.Model):
    # 默认自动创建主键ID
    data_id = models.CharField(max_length=1)  # 节点号
    temperature_data = models.CharField(max_length=3)  # 温度 度    第一位表示正负  后两位表示温度
    humidity_data = models.CharField(max_length=2)  # 湿度百分电
    wind_speed_data = models.CharField(max_length=2)  # 风速m/s
    carbon_dioxide_data = models.CharField(max_length=4)  # 二氧化碳浓度 PPM
    PVOC_data = models.CharField(max_length=4)  # PVOC清洁度   PPB
    op_time = models.DateTimeField(auto_now_add=True)  # 创建时 更新为当前时间
#控制信号表
class ControlSignalTable(models.Model):
    # 默认自动创建主键ID
    data_id = models.CharField(max_length=1)  # 节点号
    beep_signal = models.CharField(max_length=1)  # 蜂鸣器红灯报警信号
    atomizer_signal = models.CharField(max_length=1)  # 雾化工作信号 0不工作  1工作
    water_pump_signal = models.CharField(max_length=1)  # 水泵工作信号 0不工作  1工作
    heat_signal = models.CharField(max_length=1)  # 电热工作信号 0不工作  1工作
    fan_signal = models.CharField(max_length=1)  # 风扇工作信号 0不工作  1工作
    atomizer_breakdown_signal = models.CharField(max_length=1)  # 雾化故障信号 0正常  1故障
    water_pump_breakdown_signal = models.CharField(max_length=1)  # 水泵故障信号 0正常  1故障
    heat_breakdown_signal = models.CharField(max_length=1)  # 电热故障信号 0正常  1故障
    fan_breakdown_signal = models.CharField(max_length=1)  # 风扇故障信号 0正常  1故障
    dht11_breakdown_signal = models.CharField(max_length=1)  # dht11故障信号 0正常  1故障
    anemograph_breakdown_signal = models.CharField(max_length=1)  # 风速仪故障信号 0正常  1故障
    ccs811_breakdown_signal = models.CharField(max_length=1)  # ccs811故障信号 0正常  1故障
    control_type = models.CharField(max_length=1)  # 控制类型 0自动  1手动
    op_time = models.DateTimeField(auto_now_add=True)  # 创建时 更新为当前时间
#果品环境表
class FruitEnvironmentTable(models.Model):
    # 默认自动创建主键ID temperature humidity wind_speed carbon_dioxide_data PVOC_data
    fruit_name = models.CharField(max_length=5)  # 水果名字
    fruit_temperature_max = models.CharField(max_length=3)  # 水果温度最大值
    fruit_temperature_min = models.CharField(max_length=3)  # 水果温度最小值
    fruit_humidity_max = models.CharField(max_length=2)  # 水果湿度最大值
    fruit_humidity_min = models.CharField(max_length=2)  # 水果湿度最小值
    fruit_wind_speed_max = models.CharField(max_length=2)  # 水果风速最大值
    fruit_wind_speed_min = models.CharField(max_length=2)  # 水果风速最小值
    fruit_carbon_dioxide_max = models.CharField(max_length=4)  #水果二氧化碳最大值
    fruit_carbon_dioxide_min = models.CharField(max_length=4)  #水果二氧化碳最小值
    fruit_PVOC_max = models.CharField(max_length=4)    #水果PVOC最大值
    fruit_PVOC_min = models.CharField(max_length=4)    #水果PVOC最小值
    op_time = models.DateTimeField(auto_now_add=True)  # 创建时 更新为当前时间

#冷库设备安装和维修记录表
class StorageMaintainTable(models.Model):
    equipment_name = models.CharField(max_length=20) # 维修设备名字
    content= models.CharField(max_length=300)        #维修内容
    remark= models.CharField(max_length=300)         # 备注
    op_time = models.DateTimeField(auto_now_add=True)  # 创建时 更新为当前时间

#当前检测的控制水果类型  手动还是自动控制
class FruitControlType(models.Model):
    fruit_name = models.CharField(max_length=5) # 水果名字名字
    control_type= models.CharField(max_length=1)         #控制类型 0自动  1手动