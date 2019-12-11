//VIP plugin by thirteenc13
#include<cstdio>
#include<list>
#include<string>
#include<algorithm>
#include"../cmdhelper.h"
#include"../base/base.h"
#include"../serial/seral.hpp"
#include<dlfcn.h>
#include<cstdlib>
#include"../gui/gui.h"
using std::string;
using std::to_string;
using std::unordered_map;

extern "C" {
    BDL_EXPORT void mod_init(std::list<string>& modlist);
}

extern void load_helper(std::list<string>& modlist);

LDBImpl vip_data("data/new/vip");

bool isVIP(const string& name) {
    string val;
    auto succ=vip_data.Get(name,val);
    if(succ){
        if(val=="1"){
            return 1;//是
        }  
        return 0;//否
    } else {
        return 0;
    }
}

static void sendMessage(const string& msg) {
    runcmd(string("tellraw @a {\"rawtext\":[{\"text\":\"§aVIP >> "+msg+"\"}]}"));
}

void join(ServerPlayer* pl) {//VIP玩家登录提醒
    string nm=pl->getName();
    if(isVIP(nm)) {
        sendMessage("尊贵的VIP玩家 "+nm+" 加入了游戏");
    }
}

static bool chat(ServerPlayer const* pl,string& c) {
    string name=pl->getName();
    if(isVIP(name)) {
        c="§b§l"+c;//给VIP的聊天信息加上颜色
    }
    return 1;
}

void sendTPChoose(ServerPlayer* sp){//强制TP菜单
    string name=sp->getName();
    gui_ChoosePlayer(sp,"请选择目标玩家","VIP强制TP",[name](const string& dest) {
        auto xx=getSrvLevel()->getPlayer(name);
            if(xx)
            runcmdAs("vip tp "+SafeStr(dest),xx);
    });
}

void sendVIPMenu(ServerPlayer* sp) {
    string name=sp->getName();
        auto lis=new list<pair<string,std::function<void()> > >();
        lis->emplace_back(
            "调整时间到晚上",[name]{
                auto x=getSrvLevel()->getPlayer(name);
                if(x)
                    runcmdAs("vip time night",x);
            }
        );
        lis->emplace_back(
            "调整时间到早上",[name]{
                auto x=getSrvLevel()->getPlayer(name);
                if(x)
                    runcmdAs("vip time day",x);
            }
        );
        lis->emplace_back(
            "TP到指定玩家",[name]{
                auto x=getSrvLevel()->getPlayer(name);
                if(x)
                sendTPChoose((ServerPlayer*)x);
            }
        );
        string wel;
        if(isVIP(name)) {
            wel="VIP玩家:"+name+"，欢迎使用VIP功能。";
        } else {
            wel="普通玩家:"+name+"，看起来你还不是VIP，请联系服主开通VIP功能。";
        }
        gui_Buttons(sp,wel,"VIP功能",lis);
}

static void oncmd(std::vector<string>& a,CommandOrigin const & b,CommandOutput &outp) {
    ARGSZ(0)
    string prefix="§a[VIP] ";//插件消息前缀
    string name=b.getName();
    if(a.size()==0) {
        auto x=getSrvLevel()->getPlayer(name);
        sendVIPMenu(x);
        return;
    }
    if(a[0]=="help") {
        outp.error("§a---VIP help---\n/vip gui ——呼出GUI菜单\n/vip time 数值(如day night 114514) ——调整游戏时间\n/vip tp 玩家 ——直接传送到目标玩家身边\n---------");
        return;
    }
    if(a[0]=="query") {
        ARGSZ(2)
        if(isVIP(a[1])) {
            outp.success(prefix+"该玩家是VIP用户");
        } else {
            outp.error(prefix+"该玩家不是VIP用户");
        }
        return;
        
    }
    if(a[0]=="version") {
        outp.error(prefix+"Version:1.0.2 Author:thirteenc13\nhttps://github.com/thirteenc13/bdlng-VIP");
    }
    if(a[0]=="add") {//增加新的VIP
        ARGSZ(2)
        if((int)b.getPermissionsLevel()>0) {
            vip_data.Put(a[1],"1");
            outp.success(prefix+"增加了VIP用户 "+a[1]);
            sendMessage("恭喜玩家 "+a[1]+" 开通了VIP！");
            return;
        }
    }
    if(a[0]=="del") {//删除VIP
        ARGSZ(2)
        if((int)b.getPermissionsLevel()>0) {
            vip_data.Del(a[1]);
            outp.success(prefix+"删除了VIP用户 "+a[1]);
            return;
        }
    }

    if(isVIP(b.getName())==false) {//判断玩家是否是VIP，否则return
        outp.error("您没有权限使用VIP功能，请向服主赞助以获取VIP");
        return;
    }
    if(a[0]=="gui") {
        auto x=getSrvLevel()->getPlayer(name);
        sendVIPMenu(x);
    }
    if(a[0]=="time") {//设置时间
        ARGSZ(2)
        if(runcmd(string("time set "+a[1])).isSuccess()) {
            outp.success(prefix+"成功将时间设置为 "+a[1]);
        } else {
            outp.error(prefix+"时间设置失败，请检查命令");
        }
    }
    if(a[0]=="tp") {//VIP强制tp
        ARGSZ(2)
        Player* pl=NULL;
        pl=getplayer_byname(a[1]);
        if(pl==NULL) {
            outp.error(prefix+"目标玩家不在线");
            return;
        }
        TeleportA(*pl,b.getWorldPosition(), {b.getEntity()->getDimensionId()});
        outp.success(prefix+"已传送到玩家 "+a[1]+" 身边");
    }
}

/*
static void oncmd2(std::vector<string>& a,CommandOrigin const & b,CommandOutput &outp) {
    ARGSZ(1)
    string val;
    auto succ=vip_data.Get(a[0],val);
    if(succ) {
        outp.success("Pl: "+a[0]+" Db: "+val);
    } else {
        outp.error("Failure");
    }
}
*/

void mod_init(std::list<string>& modlist) {
    printf("[TVIP] Plugin loaded, Version: 1.0.2\n");
    reg_player_join(join);
    reg_chat(chat);
    register_cmd("vip",(void*)oncmd,"VIP命令");
    //register_cmd("vipdebug",(void*)oncmd2,"VIP Debug");
    load_helper(modlist);
}