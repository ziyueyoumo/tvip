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

LDBImpl vip_data("data_v2/tvip");

bool isVIP(const string& name) {
    string val;
    auto succ=vip_data.Get(name,val);
    if(succ){
        if(val=="1"){
            return 1;
        }  
        return 0;
    } else {
        return 0;
    }
}

static void sendMessage(const string& msg) {
    runcmd(string("tellraw @a {\"rawtext\":[{\"text\":\"§aVIP >> "+msg+"\"}]}"));
}

void join(ServerPlayer* pl) {//VIP user login announcement
    string nm=pl->getName();
    if(isVIP(nm)) {
        sendMessage("VIP user "+nm+" joined the game");
    }
}

static bool chat(ServerPlayer const* pl,string& c) {
    string name=pl->getName();
    if(isVIP(name)) {
        c="§b§l"+c;//Make VIP players' chat messages colorful
    }
    return 1;
}

void sendTPChoose(ServerPlayer* sp){//Teleport to target player menu
    string name=sp->getName();
    gui_ChoosePlayer(sp,"Please select a target player","Teleport to target player",[name](const string& dest) {
        auto xx=getSrvLevel()->getPlayer(name);
            if(xx)
            runcmdAs("vip tp "+SafeStr(dest),xx);
    });
}

void sendVIPMenu(ServerPlayer* sp) {
    string name=sp->getName();
        auto lis=new list<pair<string,std::function<void()> > >();
        lis->emplace_back(
            "Adjust time to night",[name]{
                auto x=getSrvLevel()->getPlayer(name);
                if(x)
                    runcmdAs("vip time night",x);
            }
        );
        lis->emplace_back(
            "Adjust time to morning",[name]{
                auto x=getSrvLevel()->getPlayer(name);
                if(x)
                    runcmdAs("vip time day",x);
            }
        );
        lis->emplace_back(
            "Teleport to target player",[name]{
                auto x=getSrvLevel()->getPlayer(name);
                if(x)
                sendTPChoose((ServerPlayer*)x);
            }
        );
        string wel;
        if(isVIP(name)) {
            wel="VIP user: "+name+"\nWelcome to VIP function.";
        } else {
            wel="Common user: "+name+"\n§cYou are not a VIP user";
        }
        gui_Buttons(sp,wel,"VIP menu",lis);
}

static void oncmd(std::vector<string>& a,CommandOrigin const & b,CommandOutput &outp) {
    ARGSZ(0)
    string prefix="§a[VIP] ";
    string name=b.getName();
    if(a.size()==0) {
        auto x=getSrvLevel()->getPlayer(name);
        sendVIPMenu(x);
        return;
    }
    if(a[0]=="v") {
        outp.error(prefix+"Version:1.0.3 Author:thirteenc13\nhttps://github.com/thirteenc13/tvip");
    }
    if(a[0]=="add") {//Add VIP
        ARGSZ(2)
        if((int)b.getPermissionsLevel()>0) {
            vip_data.Put(a[1],"1");
            outp.success(prefix+"Added VIP user "+a[1]);
            sendMessage(a[1]+" got VIP!");
            return;
        }
    }
    if(a[0]=="del") {//Delete VIP
        ARGSZ(2)
        if((int)b.getPermissionsLevel()>0) {
            vip_data.Del(a[1]);
            outp.success(prefix+"Deleted VIP user "+a[1]);
            return;
        }
    }

    if(isVIP(b.getName())==false) {//Determine if the player is VIP
        outp.error("You have no VIP permission.");
        return;
    }
    if(a[0]=="gui") {
        auto x=getSrvLevel()->getPlayer(name);
        sendVIPMenu(x);
    }
    if(a[0]=="time") {//Set time
        ARGSZ(2)
        if(runcmd(string("time set "+a[1])).isSuccess()) {
            outp.success(prefix+"Set time to night "+a[1]);
        } else {
            outp.error(prefix+"Setting failed, please check the command.");
        }
    }
    if(a[0]=="tp") {//VIP teleport
        ARGSZ(2)
        Player* pl=NULL;
        pl=getplayer_byname(a[1]);
        if(pl==NULL) {
            outp.error(prefix+"Target player is offline.");
            return;
        }
        TeleportA(*getplayer_byname(b.getName()),pl->getPos(),{pl->getDimensionId()});
        outp.success(prefix+"Teleported you to "+a[1]);
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
    printf("[TVIP] Plugin loaded, Version: 1.0.3\n");
    reg_player_join(join);
    reg_chat(chat);
    register_cmd("vip",(void*)oncmd,"VIP commands");
    //register_cmd("vipdebug",(void*)oncmd2,"VIP Debug");
    load_helper(modlist);
}