
from django.shortcuts import render
from django.http import JsonResponse
from django.forms.models import model_to_dict
from django.shortcuts import render, HttpResponse, redirect
import json
from user_app import models

def login(request):
    if request.method == "GET":
        return render(request, 'login.html')
    elif request.method == "POST":
        return_data = {'status': True, 'error': None}  #usercode userpassword remember_passward
        operate = request.POST.get('operate', None)  # 获取需要的操作
        if operate=="login":
            user_code = request.POST.get('usercode', None)  # 沒有则为空
            password = request.POST.get('userpassword', None)
            remember = request.POST.get('remember_passward', None)  # 这是Ture 记住密码or falase不记住
            print("账号:", user_code)
            print("密码:", password)
            print("记住密码:", remember)
            obj = models.UserTable.objects.filter(user_code=user_code).first()  # 找到用户  且用户名
            if obj:
                if obj.user_password == password:
                    request.session['is_login'] = True
                    request.session['user_code'] = user_code
                    request.session.set_expiry(24 * 60 * 60)  # 一天免登陆
                    if remember == "True":
                        # session中设置值
                        request.session.set_expiry(14*24*60 * 60)  # 2周内免登陆
                        print("设置了两周免登陆")
                else:
                    return_data['status'] = False
                    return_data['error'] = "密码错误请重新输入"
            else:
                return_data['status'] = False
                return_data['error'] = "账号不存在"
        elif operate=="register":
            register_usercode = request.POST.get('register_usercode', None)  # 沒有则为空
            register_userpassword = request.POST.get('register_userpassword', None)
            register_username = request.POST.get('register_username', None)
            register_usermail = request.POST.get('register_usermail', None)
            register_userphone = request.POST.get('register_userphone', None)
            obj = models.UserTable.objects.filter(user_code=register_usercode).first()  # 找到用户  且用户名
            if obj:
                return_data['status'] = False
                return_data['error'] = "账号已存在"
            else:
                models.UserTable.objects.create(user_code=register_usercode, user_password=register_userpassword,
                                                user_name=register_username,user_mail=register_usermail,user_phone=register_userphone,
                                                user_type="0",remark=None)
        else:
            return_data['status'] = False
            return_data['error'] = '未知错误'
        return HttpResponse(json.dumps(return_data))
    else:
        return HttpResponse("该页面还在准备中@@")

def home(request):
    if request.method == "GET":
        # session中获取值            #登录成功后的操作
        if request.session.get('is_login', False):
            user_code=request.session['user_code']   #用户账号
            obj = models.UserTable.objects.filter(user_code=user_code).first()  #查找用户相关信息
            return render(request,'Home.html',{'user_name':obj.user_name})
        else:
            return render(request, 'login.html')
    else:
        return HttpResponse("该页面还在准备中@@")

def bim(request):
    if request.method == "GET":
        # session中获取值            #登录成功后的操作
        if request.session.get('is_login', False):
            user_code=request.session['user_code']   #用户账号
            return render(request,'BIM.html')
        else:
            return render(request, 'login.html')
    elif request.method == "POST":
        login_user_code = request.session['user_code']  # 获取操操作户账号
        return_data = {'status': True, 'data': None}
        operate = request.POST.get('operate', None)  # 获取需要的操作

        if operate=="auto_or_manual":
            control_type=request.POST.get('control_type', None)   #控制类型  true 代表手动控制  false代表自动控制
            if control_type=="false":
                models.FruitControlType.objects.filter(id=1).update(control_type=0)   #修改数据库为自动控制
            else:
                models.FruitControlType.objects.filter(id=1).update(control_type=1)   #修改数据库为手动控制
                manual_control_list=["0","0","0","0","0","0","0","0","0","0","0","0"]
                if request.POST.get('beep_signal', None)=="true":
                    manual_control_list[0]="1"
                if request.POST.get('atomizer_signal', None)=="true":
                    manual_control_list[1]="1"
                if request.POST.get('water_pump_signal', None)=="true":
                    manual_control_list[2]="1"
                if request.POST.get('heat_signal', None)=="true":
                    manual_control_list[3]="1"
                if request.POST.get('fan_signal', None)=="true":
                    manual_control_list[4]="1"
                if request.POST.get('atomizer_breakdown_signal', None)=="true":
                    manual_control_list[5]="1"
                if request.POST.get('water_pump_breakdown_signal', None)=="true":
                    manual_control_list[6]="1"
                if request.POST.get('heat_breakdown_signal', None)=="true":
                    manual_control_list[7]="1"
                if request.POST.get('fan_breakdown_signal', None)=="true":
                    manual_control_list[8]="1"
                if request.POST.get('dht11_breakdown_signal', None)=="true":
                    manual_control_list[9]="1"
                if request.POST.get('anemograph_breakdown_signal', None)=="true":
                    manual_control_list[10]="1"
                if request.POST.get('ccs811_breakdown_signal', None)=="true":
                    manual_control_list[11]="1"
                #print(manual_control_list)
                #将控制的数据存入数据库中
                models.ControlSignalTable.objects.create(data_id="1",beep_signal=manual_control_list[0],atomizer_signal=manual_control_list[1],
                                                            water_pump_signal=manual_control_list[2],heat_signal=manual_control_list[3],
                                                            fan_signal=manual_control_list[4],atomizer_breakdown_signal=manual_control_list[5],
                                                            water_pump_breakdown_signal=manual_control_list[6],heat_breakdown_signal=manual_control_list[7],
                                                            fan_breakdown_signal=manual_control_list[8],dht11_breakdown_signal=manual_control_list[9],
                                                            anemograph_breakdown_signal=manual_control_list[10],ccs811_breakdown_signal=manual_control_list[11],
                                                            control_type="1")
            return JsonResponse(return_data, safe=False)


        elif operate=="refresh_data":
            return_list=[]
            sensor_list=[]
            control_list=[]
            obj = models.SensorDataTable.objects.all().order_by('-id')[:10]       #选出环境数据最后10条  filter(时间字段名称__year=2005)
            if len(obj)==0:
                return_data['status']=False
            else:
                for line in obj:
                    jsonstr = model_to_dict(line)       #对象数据转字典
                    jsonstr["op_time"] = str(line.op_time)
                    sensor_list.append(jsonstr)
            obj = models.ControlSignalTable.objects.filter(control_type="0").last()    #选出最后一条自动控制数据
            control_list.append( model_to_dict(obj) ) # 对象数据转字典
            return_list.append(sensor_list)
            return_list.append(control_list)
            return_data['data'] =  return_list                      # return_list  第一条列表存储的是10条环境数据的字典  第二条列表存的一条数据的字典
            #print(return_list)
            return JsonResponse(return_data, safe=False)  # 前端后端  都不需要序列号 和反序列化操作 return_data可以为字典列表元组 不可以为对象

        elif operate=="add_fruit":
            input_fruit_name=request.POST.get('input_fruit_name', None)   #水果名
            input_temperature_max=request.POST.get('input_temperature_max', None)    #温度最大值
            input_temperature_min=request.POST.get('input_temperature_min', None)
            input_humidity_max=request.POST.get('input_humidity_max', None)
            input_humidity_min=request.POST.get('input_humidity_min', None)
            input_wind_speed_max=request.POST.get('input_wind_speed_max', None)
            input_wind_speed_min=request.POST.get('input_wind_speed_min', None)
            input_carbon_dioxide_max=request.POST.get('input_carbon_dioxide_max', None)
            input_carbon_dioxide_min=request.POST.get('input_carbon_dioxide_min', None)
            input_PVOC_max=request.POST.get('input_PVOC_max', None)
            input_PVOC_min=request.POST.get('input_PVOC_min', None)
            if input_fruit_name=="" or input_temperature_max==""  or input_temperature_min==""  or input_humidity_max==""  or input_humidity_min==""  or input_wind_speed_max==""  or input_wind_speed_min==""  or input_carbon_dioxide_max==""  or input_carbon_dioxide_min==""  or input_PVOC_max==""  or input_PVOC_min==""  :
                return_data['status'] = False
                return_data['data'] = "输入不能不空"
                return JsonResponse(return_data, safe=False)
            if int(input_temperature_max)<int(input_temperature_min) or int(input_humidity_max)<int(input_humidity_min) or int(input_wind_speed_max)<int(input_wind_speed_min) or int(input_carbon_dioxide_max)<int(input_carbon_dioxide_min) or int(input_PVOC_max)<int(input_PVOC_min):
                return_data['status'] = False
                return_data['data'] = "最大值必须大于最小值"
                return JsonResponse(return_data, safe=False)
            if len(models.FruitEnvironmentTable.objects.filter(fruit_name=input_fruit_name))>0:
                return_data['status'] = False
                return_data['data'] = "该果品已存在"
                return JsonResponse(return_data, safe=False)
            models.FruitEnvironmentTable.objects.create(fruit_name=input_fruit_name, fruit_temperature_max=input_temperature_max,fruit_temperature_min=input_temperature_min,
                    fruit_humidity_max=input_humidity_max,fruit_humidity_min=input_humidity_min,fruit_wind_speed_max=input_wind_speed_max,fruit_wind_speed_min=input_wind_speed_min,
                    fruit_carbon_dioxide_max=input_carbon_dioxide_max,fruit_carbon_dioxide_min=input_carbon_dioxide_min,fruit_PVOC_max=input_PVOC_max,fruit_PVOC_min=input_PVOC_min)
            dic_FruitEnvironment_list = []
            obj = models.FruitEnvironmentTable.objects.filter()
            for line in obj:
                jsonstr = model_to_dict(line)       #对象数据转字典
                dic_FruitEnvironment_list.append(jsonstr)
            print(dic_FruitEnvironment_list)
            return_data['data'] =  dic_FruitEnvironment_list
            return JsonResponse(return_data, safe=False)
        elif operate=="load_fruit":
            obj = models.FruitEnvironmentTable.objects.filter()
            return_fruit_list = []
            for line in obj:
                jsonstr = model_to_dict(line)  # 对象数据转字典
                return_fruit_list.append(jsonstr["fruit_name"])
            obj = models.FruitControlType.objects.filter()
            for line in obj:
                jsonstr = model_to_dict(line)  # 对象数据转字典
                return_fruit_list.append(jsonstr["fruit_name"])
            #print(return_fruit_list)
            return_data['data'] =  return_fruit_list
            return JsonResponse(return_data, safe=False)

        elif operate=="change_fruit_type":
            change_fruit_type= request.POST.get('change_fruit_type', None)  # 获取新的水果类型
            models.FruitControlType.objects.filter(id=1).update(fruit_name=change_fruit_type)
            return JsonResponse(return_data, safe=False)
    else:
        return HttpResponse("该页面还在准备中@@")

def history(request):
    if request.method == "GET":
        # session中获取值            #登录成功后的操作
        if request.session.get('is_login', False):
            return render(request,'History.html')
        else:
            return render(request, 'login.html')


    elif request.method == "POST":
        login_user_code = request.session['user_code']  # 获取操操作户账号
        return_data = {'status': True, 'data': None}
        operate = request.POST.get('operate', None)  # 获取需要的操作

        if operate=="search":
            freezer_id=request.POST.get('freezer_id', None)   # 冷库ID号
            find_type=request.POST.get('find_type', None)   # 需要查询的是0传感器记录还是  1控制记录
            year=request.POST.get('year', None)   #all代表全部
            month=request.POST.get('month', None)   #all代表全部
            day=request.POST.get('day', None)   #all代表全部
            hour=request.POST.get('hour', None)   #all代表全部
            minute=request.POST.get('minute', None)   #all代表全部
            second=request.POST.get('second', None)   #all代表全部
            state=request.POST.get('state', None)   # 全部正常还是报警    控制数据时有效
            dic_user_list = []
            if find_type=="0":
                #冷藏库号+年月日时分秒查询数据  返回
                if year=="all" and month=="all" and day=="all" and hour=="all" and minute=="all" and second=="all":
                    obj = models.SensorDataTable.objects.filter(data_id=freezer_id)
                elif  month=="all" and day=="all" and hour=="all" and minute=="all" and second=="all":
                    obj = models.SensorDataTable.objects.filter(data_id=freezer_id,op_time__year=year)
                elif day=="all" and hour=="all" and minute=="all" and second=="all":
                    obj = models.SensorDataTable.objects.filter(data_id=freezer_id,op_time__year=year,op_time__month=month)
                elif hour=="all" and minute=="all" and second=="all":
                    obj = models.SensorDataTable.objects.filter(data_id=freezer_id,op_time__year=year,op_time__month=month,op_time__day=day)
                elif minute=="all" and second=="all":
                    obj = models.SensorDataTable.objects.filter(data_id=freezer_id,op_time__year=year,op_time__month=month,op_time__day=day,op_time__hour=hour)
                elif second=="all":
                    obj = models.SensorDataTable.objects.filter(data_id=freezer_id,op_time__year=year,op_time__month=month,op_time__day=day,op_time__hour=hour,op_time__minute=minute)
                elif year!="all" and month!="all" and day!="all" and hour!="all" and minute!="all" and second!="all":
                    obj = models.SensorDataTable.objects.filter(data_id=freezer_id,op_time__year=year,op_time__month=month,op_time__day=day,op_time__hour=hour,op_time__minute=minute)
                else:
                    obj=""
                if len(obj) == 0:
                    return_data['status'] = False
                else:
                    for line in obj:
                        jsonstr = model_to_dict(line)  # 对象数据转字典
                        jsonstr["op_time"] = str(line.op_time)     #添加操作时间
                        dic_user_list.append(jsonstr)
                return_data['data'] = dic_user_list
                return JsonResponse(return_data, safe=False)

            elif find_type=="1":
                #冷藏库号+年月日时分秒查询数据+ 报警状态  state  0正常  1报警  all表示全部
                if state=="all":
                    if year == "all" and month == "all" and day == "all" and hour == "all" and minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id)
                    elif month == "all" and day == "all" and hour == "all" and minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year)
                    elif day == "all" and hour == "all" and minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month)
                    elif hour == "all" and minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month, op_time__day=day)
                    elif minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month, op_time__day=day,
                                                                    op_time__hour=hour)
                    elif second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month, op_time__day=day,
                                                                    op_time__hour=hour, op_time__minute=minute)
                    elif year != "all" and month != "all" and day != "all" and hour != "all" and minute != "all" and second != "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month, op_time__day=day,
                                                                    op_time__hour=hour, op_time__minute=minute)
                    else:
                        obj = ""
                else:
                    if year == "all" and month == "all" and day == "all" and hour == "all" and minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id,beep_signal=state)
                    elif month == "all" and day == "all" and hour == "all" and minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year)
                    elif day == "all" and hour == "all" and minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month)
                    elif hour == "all" and minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month, op_time__day=day)
                    elif minute == "all" and second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month, op_time__day=day,
                                                                    op_time__hour=hour)
                    elif second == "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month, op_time__day=day,
                                                                    op_time__hour=hour, op_time__minute=minute)
                    elif year != "all" and month != "all" and day != "all" and hour != "all" and minute != "all" and second != "all":
                        obj = models.ControlSignalTable.objects.filter(data_id=freezer_id, op_time__year=year,
                                                                    op_time__month=month, op_time__day=day,
                                                                    op_time__hour=hour, op_time__minute=minute)
                    else:
                        obj = ""
                if len(obj) == 0:
                    return_data['status'] = False
                else:
                    for line in obj:
                        jsonstr = model_to_dict(line)  # 对象数据转字典
                        jsonstr["op_time"] = str(line.op_time)     #添加操作时间
                        dic_user_list.append(jsonstr)
                return_data['data'] = dic_user_list
                return JsonResponse(return_data, safe=False)
    else:
        return HttpResponse("该页面还在准备中@@")





def person(request):
    if request.method == "GET":
        if request.session.get('is_login', False):
            #登录成功后的操作
            user_code=request.session['user_code']   #获取操操作户账号
            obj = models.UserTable.objects.filter(user_code=user_code).first()  #查找用户相关信息
            return_dic={'user_code':obj.user_code,
                        'user_name': obj.user_name,
                        'user_mail': obj.user_mail,
                        'user_phone': obj.user_phone,
                        'remark': obj.remark
            }
            return_dic['user_mail']='无' if obj.user_mail is None else obj.user_mail
            return_dic['user_phone']='无' if obj.user_phone is None else obj.user_phone
            return_dic['remark']='无' if obj.remark is None else obj.remark
            return render(request,'Person.html',return_dic)
        else:
            return render(request, 'login.html')
    elif request.method == "POST":
        ret = {'status': True, 'error': None}
        user_code = request.session['user_code']  # 获取操作用户账号
        obj = models.UserTable.objects.filter(user_code=user_code).first()
        up_data_dic = {'user_name': obj.user_name,
                       'user_password': obj.user_password,
                      'user_mail': obj.user_mail,
                      'user_phone': obj.user_phone
                      }
        user_name = request.POST.get('user_name', None)  # 沒有则为空
        old_password = request.POST.get('old_password', None)
        new_password = request.POST.get('new_password', None)
        again_password = request.POST.get('again_password', None)
        user_phone = request.POST.get('user_phone', None)
        user_mail = request.POST.get('user_mail', None)
        change_password = request.POST.get('change_password', None)

        if len(user_name)==0:
            ret['status'] = False
            ret['error'] = '昵称不能为空'
            return HttpResponse(json.dumps(ret))
        up_data_dic["user_name"] = user_name

        if change_password == "true":
            if old_password == obj.user_password:
                if new_password!=again_password:
                    ret['status'] = False
                    ret['error'] = '两次密码输入不一致'
                    return HttpResponse(json.dumps(ret))
                elif len(new_password)==0:
                    ret['status'] = False
                    ret['error'] = '密码不能为空'
                    return HttpResponse(json.dumps(ret))
            else:
                ret['status'] = False
                ret['error'] = '原始密码错误'
                return HttpResponse(json.dumps(ret))
            up_data_dic["user_password"]=new_password

        if  user_phone== "无" or user_phone== "":
            up_data_dic["user_phone"] = None
        else:
            up_data_dic["user_phone"] = user_phone
        if  user_mail== "无" or user_mail== "":
            up_data_dic["user_mail"] = None
        else:
            up_data_dic["user_mail"] = user_mail

        models.UserTable.objects.filter(user_code=user_code).update(user_name=up_data_dic["user_name"],user_mail=up_data_dic["user_mail"],
                                                                    user_phone=up_data_dic["user_phone"],user_password=up_data_dic["user_password"])
        return HttpResponse(json.dumps(ret))

def logout(request):
    request.session.clear()
    return redirect('/login/')

