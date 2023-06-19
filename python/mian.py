# 读取数据库更快水果类型自动控制返回的数据

# 1.传感器数据入库
# 2.读取user_app_fruitcontroltype表获取 控制类型和存储水果   fruit_name、control_type#控制类型 0自动  1手动
# 3.若为自动则到  user_app_fruitenvironmenttable表取出各种数据的阈值
# 4.做出返回故障和控制信号后  存入user_app_controlsignaltable表中（注意类型为自动控制）
# 5.将控制数据返回下位机     //////////////若为手动控制，则到user_app_controlsignaltable表中取出最后一条手动控制数据，返回下位机

import socketserver  # 支持多个客户端   无限发送
import pymysql
import time

fan_fault_detect_time = 2  # 风扇故障检测  选取上面的数据条数  时间=条数/0 如20则检测10是的数据  风扇   最大支持100
heat_fault_detect_time = 50
bump_fault_detect_time = 50
humi_fault_detect_time = 50


class MyTCPHandler(socketserver.BaseRequestHandler):
    def handle(self):  # 跟客服端所以的交互 都在这处理
        while True:
            try:  # 客户端断开时 抓异常处理
                self.data = self.request.recv(4096).strip()  # self是客户请求实例化 接收的数据
                # 存入数据库
                receive_list = self.data.decode('utf-8').split('#')  # 依次是节点号  温度  湿度 风速 co2 pvoc
                print(receive_list)  # 分离字符串到列表

                if len(receive_list) == 6:
                    # 数据入库
                    con = pymysql.connect(host='localhost', port=3307, user='root', passwd='123456',
                                          db='django_freezer_db')
                    cursor = con.cursor()
                    cursor.execute(
                        "insert into user_app_sensordatatable (data_id,temperature_data,humidity_data,wind_speed_data,carbon_dioxide_data,PVOC_data,op_time) values ('%s','%s','%s','%s','%s','%s','%s');" %
                        (receive_list[0], receive_list[1], receive_list[2], receive_list[3], receive_list[4],
                         receive_list[5],
                         time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())))
                    con.commit()
                    # 查询水果当前控制水果信息和是否是自动控制
                    cursor.execute("select * from user_app_fruitcontroltype order by ID limit 0,1;")  # 查询第一条数据
                    fruitcontroltype_list = cursor.fetchone()  # 取一行的元组
                    con.commit()
                    # 如果是自动控制
                    if fruitcontroltype_list[2] == '0':
                        # 取对应水果环境信息
                        cursor.execute("SELECT * FROM user_app_fruitenvironmenttable WHERE fruit_name='%s';" % (
                            fruitcontroltype_list[1]))
                        fruitenvironment_list = cursor.fetchone()
                        # print(fruitenvironment_list)  # 没有数据就是None  可能报错哦############################################
                        returen_control_list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]  # 返回控制信号   初始都是0
                        if int(receive_list[1]) > int(fruitenvironment_list[2]):
                            returen_control_list[2] = 1  # 温度过高水泵打开
                            # print("温度过高")
                        elif int(receive_list[1]) < int(fruitenvironment_list[3]):
                            returen_control_list[3] = 1  # 温度过低电热打开
                            # print("温度过低")
                        if int(receive_list[2]) > int(fruitenvironment_list[4]):
                            returen_control_list[3] = 1  # 湿度过高电热打开
                            # print("湿度过高")
                        elif int(receive_list[2]) < int(fruitenvironment_list[5]):
                            returen_control_list[1] = 1  # 湿度过低雾化打开
                            # print("湿度过低")
                        if int(receive_list[3]) > int(fruitenvironment_list[6]):
                            returen_control_list[4] = 0  # 风速过高风扇关闭
                            # print("风速过高")
                        elif int(receive_list[2]) < int(fruitenvironment_list[7]):
                            returen_control_list[4] = 1  # 风速过低风扇打开
                            # print("风速过低")
                        if int(receive_list[4]) > int(fruitenvironment_list[8]) or int(receive_list[4]) < int(
                                fruitenvironment_list[9]) or int(receive_list[5]) > int(
                            fruitenvironment_list[10]) or int(receive_list[5]) < int(fruitenvironment_list[11]):
                            returen_control_list[11] = 1  # CCS811故障
                            returen_control_list[0] = 1  # 打开蜂鸣器报警
                            # print("ccs811故障")
                        if int(receive_list[3]) > 5:
                            returen_control_list[10] = 1  # 风速仪数值异常故障
                            returen_control_list[0] = 1  # 打开蜂鸣器报警
                            # print("风速仪异常")
                        if int(receive_list[1]) > 100 or int(receive_list[2]) > 100:
                            returen_control_list[9] = 1  # 温湿度超过量程视为故障
                            returen_control_list[0] = 1  # 打开蜂鸣器报警
                            # print("温湿度模块故障")
                        # 取出数据库自动控制信号数据最后20行 即是10s   不足20条数据不处理   连续风扇开启 ，则视为风扇故障
                        cursor.execute(
                            "select * from user_app_controlsignaltable where control_type=0 order by ID desc limit 0,100;")  # 数据库自动控制的最后100行数据
                        last_control_data = cursor.fetchall()  # 二维元组  最后一行在前   第一行在后
                        cursor.execute(
                            "select * from user_app_sensordatatable order by ID desc limit 0,100;")  # 数据库自动控制的最后100行数据
                        last_envir_data = cursor.fetchall()  # 最后几行环境数据
                        count = 0
                        for i in range(fan_fault_detect_time):
                            if int(last_control_data[i][6]) == 1:  # 风扇控制信号为启动 连续一段时间
                                count = count + 1
                        if count == fan_fault_detect_time and int(receive_list[3]) == 0:  # 风速为0
                            returen_control_list[8] = 1  # 判定为风扇故障
                            returen_control_list[0] = 1  # 打开蜂鸣器报警
                        count = 0
                        for i in range(heat_fault_detect_time):
                            if int(last_control_data[i][5]) == 1:  # 加热控制信号为启动 连续一段时间
                                count = count + 1
                        if count == heat_fault_detect_time and int(last_envir_data[0][2]) - int(
                                last_envir_data[heat_fault_detect_time][2]) == 0:  # 一直在加热 但是温度没有变化
                            returen_control_list[7] = 1  # 判定为加热片故障
                            returen_control_list[0] = 1  # 打开蜂鸣器报警
                        count = 0
                        for i in range(bump_fault_detect_time):
                            if int(last_control_data[i][4]) == 1:  # 水泵控制信号为启动 连续一段时间
                                count = count + 1
                        if count == bump_fault_detect_time and int(last_envir_data[0][2]) - int(
                                last_envir_data[bump_fault_detect_time][2]) == 0:  # 一直在水泵模拟空调制冷 但是温度没有变化
                            returen_control_list[6] = 1  # 判定为水泵故障
                            returen_control_list[0] = 1  # 打开蜂鸣器报警
                        count = 0
                        for i in range(humi_fault_detect_time):
                            if int(last_control_data[i][3]) == 1:  # 雾化控制信号为启动 连续一段时间
                                count = count + 1
                        if count == humi_fault_detect_time and int(last_envir_data[0][3]) - int(
                                last_envir_data[humi_fault_detect_time][3]) == 0:  # 一直在水泵模拟空调制冷 但是湿度没有变化
                            returen_control_list[5] = 1  # 判定为雾化片故障
                            returen_control_list[0] = 1  # 打开蜂鸣器报警
                        # 将返回控制的 控制信号存入数据库
                        cursor.execute(
                            "insert into user_app_controlsignaltable (data_id,beep_signal ,atomizer_signal ,water_pump_signal,heat_signal ,fan_signal,atomizer_breakdown_signal ,water_pump_breakdown_signal,heat_breakdown_signal ,fan_breakdown_signal,dht11_breakdown_signal ,anemograph_breakdown_signal,ccs811_breakdown_signal ,control_type,op_time) values ('1','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s');" %
                            (returen_control_list[0], returen_control_list[1], returen_control_list[2],
                             returen_control_list[3],
                             returen_control_list[4], returen_control_list[5], returen_control_list[6],
                             returen_control_list[7],
                             returen_control_list[8], returen_control_list[9], returen_control_list[10],
                             returen_control_list[11], '0',
                             time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())))
                        con.commit()
                        con.close()
                        for i in range(4):
                            if (returen_control_list[i * 3 + 0] + returen_control_list[i * 3 + 1] +
                                returen_control_list[i * 3 + 2]) % 2 == 0:
                                returen_control_list[12 + i] = 0
                            else:
                                returen_control_list[12 + i] = 1
                        returen_control_str = ''.join('%s' % id for id in returen_control_list)
                        self.request.send(returen_control_str.encode("utf-8"))
                    else:
                        # 手动控制
                        # 到user_app_controlsignaltable表中取出最后一条手动控制数据，返回下位机
                        sql = "select * from user_app_controlsignaltable where control_type=1 order by ID desc limit 0,1;"  # 数据库的手动控制最后一行数据
                        cursor.execute(sql)
                        manual_control = cursor.fetchone()  # 取一行的元组
                        con.close()
                        # 控制0-4：蜂鸣器，雾化，水泵，电热，风扇   故障5-11：雾化，水泵，电热，风扇，dht11,风速仪，CCS811  校检12-15  三位奇偶校检
                        returen_control_list = [int(manual_control[2]), int(manual_control[3]), int(manual_control[4]),
                                                int(manual_control[5]), int(manual_control[6]), int(manual_control[7]),
                                                int(manual_control[8]), int(manual_control[9]), int(manual_control[10]),
                                                int(manual_control[11]), int(manual_control[12]),
                                                int(manual_control[13]), 0, 0, 0, 0]  # 仅加热
                        for i in range(4):
                            if (returen_control_list[i * 3 + 0] + returen_control_list[i * 3 + 1] +
                                returen_control_list[i * 3 + 2]) % 2 == 0:
                                returen_control_list[12 + i] = 0
                            else:
                                returen_control_list[12 + i] = 1
                        returen_control_str = ''.join('%s' % id for id in returen_control_list)
                        self.request.send(returen_control_str.encode("utf-8"))
            except ConnectionResetError as e:  #
                print("err", e)
                break


if __name__ == "__main__":
    HOST, PORT = '172.20.10.4', 8080
    server = socketserver.ThreadingTCPServer((HOST, PORT), MyTCPHandler)  # 传ip 传类      ThreadingTCPServer多线程
    server.serve_forever()
