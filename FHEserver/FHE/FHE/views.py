from django.http import HttpResponse
from django.shortcuts import render
from user.models import User
import os


def login(request):
    info = {'status': ''}
    return render(request, 'login.html', info)


def download(request):
    vid = {'title': 'Operating...'}
    return render(request, "be.html", vid)


def operate(request):
    video = request.FILES.get("video", None)
    if not video:
        return HttpResponse("No Uploaded Video")
    vid = {}
    title = 'Blind Extraction for ' + video.name
    vid['title'] = title
    vid['res_name'] = ('res_' + video.name)
    vid['ext_name'] = ('ext_' + video.name)
    return render(request, "be.html", vid)


def be(request):
    vid = {}
    vid['title'] = ''
    return render(request, "be.html", vid)


def loginstat(request):
    info = {}
    vid = {}
    num = request.POST.get('num')
    pwd = request.POST.get('pwd')
    if num == '' or pwd == '':
        info['status'] = 'Please input account and password'
        return render(request, 'login.html', info)
    else:
        if User.objects.filter(user_name=num).exists():
            if User.objects.filter(user_name=num, user_pwd=pwd).exists():
                vid = {'res_name': '', 'ext_name': '', 'title': ''}
                return render(request, 'be.html', vid)
            else:
                info['status'] = 'User name or password is wrong'
                return render(request, 'login.html', info)
        else:
            usr = User(user_name=num, user_pwd=pwd)
            usr.save()
            vid = {'res_name': '', 'ext_name': '', 'title': ''}
            return render(request, 'be.html', vid)
