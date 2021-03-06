//
//  ForgetPassLayer.cpp
//  ZJH
//
//  Created by apple on 16/7/11.
//
//

#include "ForgetPassLayer.hpp"
#define CELLH2  160
#define ITERH_MAX   30
#define ITERH_MIN   3

bool ForgetPassLayer::init()
{
    if (!BaseLayer::init())
    {
        return false;
    }
    setTitle("忘记密码");
    setReturn_img("Head/title-icon1.png", "Head/title-icon1_press.png", [=](){
        this->disappear();
    });
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float H = visibleSize.height - getHeadH() - 25;
    
    setBackground(Color4B(0x1d, 0x1d, 0x1e, 255));
    
    Button *nextBtn = Button::create("Mine/btn_yellow_big.png","Mine/btn_yellow_big_check.png", "Mine/btn_yellow_big_check.png");
    nextBtn->setScale9Enabled(true);
    nextBtn->setContentSize(Size(960, 120));
    nextBtn->setPosition(Vec2(visibleSize.width/2, H - 480));
    this->addChild(nextBtn);
    nextBtn->setTitleText("下一步");
    nextBtn->setTitleFontSize(40);
    nextBtn->setTitleColor(Color3B(0x65, 0x40, 0x02));
    nextBtn->setTitleFontName("");
    nextBtn->addTouchEventListener(CC_CALLBACK_2(ForgetPassLayer::nextOn, this));
    
    for (int i = 0; i < 2; i++)
    {
        auto cell = Layout::create();
        cell->ignoreAnchorPointForPosition(false);
        cell->setAnchorPoint(Vec2(0, 1));
        cell->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
        cell->setBackGroundColor(Color3B(0x26, 0x26, 0x29));
        cell->setContentSize(Size(visibleSize.width, 180));
        this->addChild(cell);
        
        Text *title = Text::create("", "", 40);
        title->setPosition(Vec2(210, 90));
        title->setTextColor(Color4B(0xde, 0xde, 0xdf, 255));
        title->setAnchorPoint(Vec2(1, 0.5));
        title->setTextHorizontalAlignment(cocos2d::TextHAlignment::LEFT);
        cell->addChild(title);
        
        EditBox* edit = EditBox::create(Size(800, 100), "Mine/bar_sendtxt.png");
        edit->setAnchorPoint(Vec2(0, 0.5));
        edit->setMaxLength(11);
        edit->setInputMode(EditBox::InputMode::NUMERIC);
        edit->setPosition(Vec2(235, 90));
        edit->setFontColor(Color3B(0x66, 0x66, 0x66));
        edit->setPlaceholderFontColor(Color3B(0xb2, 0xb2, 0xb2));
        edit->setFontSize(45);
        edit->setPlaceholderFontSize(40);
        cell->addChild(edit);
        
        switch (i)
        {
            case 0:
                edit1 = edit;
                cell->setPosition(Vec2(0, H));
                title->setString("手机号");
                edit->setPlaceHolder("请输入手机号");
                break;
            case 1:
                edit2 = edit;
                cell->setPosition(Vec2(0, H - 187));
                title->setString("验证码");
                edit->setPlaceHolder("请输入短信验证码");
                edit->setContentSize(Size(490, 100));
                
                msgBtn = Button::create("Mine/btn_yellow_small.png","Mine/btn_yellow_small_check.png", "Mine/btn_yellow_small_check.png");
                msgBtn->setScale9Enabled(true);
                msgBtn->setContentSize(Size(270, 90));
                msgBtn->setPosition(Vec2(visibleSize.width - 160, 90));
                cell->addChild(msgBtn);
                msgBtn->setTitleText("发送验证码");
                msgBtn->setTitleFontSize(36);
                msgBtn->setTitleColor(Color3B(0x65, 0x40, 0x02));
                msgBtn->setTitleFontName("");
                msgBtn->addTouchEventListener(CC_CALLBACK_2(ForgetPassLayer::postCheckNumOn, this));
                break;
        }
    }

    return true;
}

//发送验证码
void ForgetPassLayer::postCheckNumOn(Ref *pSender, Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::BEGAN)
    {
        string phone = edit1->getText();
        if (phone == "")
        {
            PlatformHelper::showToast("手机号码不能为空！");
            return;
        }
        Button *btn = (Button *)pSender;
        btn->setEnabled(false);
        
        /*
         sendType:
         (1,"注册类型"),	(2, "手机绑定"),	(3, "修改手机"),	(4, "重置密码"),
         (5, "实名认证"),	(6, "系统警报"),	(7, "忘记密码"),    (8,"登录类型") (9,"美女认证")
         */
        Json::Value json;
        json["phone"] =  phone;
        json["sendType"] = 7;
        
        CCHttpAgent::getInstance()->sendHttpPost([=](std::string tag){
            CCHttpPacket* loginPacket = CCHttpAgent::getInstance()->packets[tag];
            if (this->getReferenceCount() <= 0 || this->getReferenceCount() > 10)return;
            
            if (loginPacket->status != 3)
            {
                btn->setEnabled(true);
                PlatformHelper::showToast("网络链接失败，请稍后再试");
                return;
            }
            
            if (loginPacket->resultIsOK())
            {
                Json::Value data = loginPacket->recvVal["resultMap"];
                secend = 120;
                msgBtn->setTitleText("发送中...");
                this->unscheduleUpdate();
                this->schedule(schedule_selector(ForgetPassLayer::timeDown), 1.0);
            }else
            {
                btn->setEnabled(true);
                if (!loginPacket->recvVal["message"].isNull() && loginPacket->recvVal["message"].isString())
                {
                    PlatformHelper::showToast(loginPacket->recvVal["message"].asCString());
                }
                
            }
        },"user/getPhoneCode",json.toStyledString(),"getPhoneCode");
        
    }
}

void ForgetPassLayer::timeDown(float dt)
{
    if (secend >= 0)
    {
        char s[128];
        sprintf(s, "%ds后重新发送", secend);
        msgBtn->setTitleText(s);
        msgBtn->setEnabled(false);
    }else
    {
        msgBtn->setTitleText("发送验证码");
        msgBtn->setEnabled(true);
        this->unscheduleUpdate();
    }
    secend--;
}
//忘记密码下一步
void ForgetPassLayer::nextOn(Ref *pSender, Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        string code = edit2->getText();
        if (code == "")
        {
            PlatformHelper::showToast("验证码为空");
            return;
        }
        Json::Value json;
        json["phone"] = edit1->getText();
        json["phoneCode"] = edit2->getText();
        json["sendType"] = 7;
    
        CCHttpAgent::getInstance()->sendHttpPost([=](std::string tag){
            CCHttpPacket* loginPacket = CCHttpAgent::getInstance()->packets[tag];
            if (this->getReferenceCount() <= 0 || this->getReferenceCount() > 10)return;
            
            if (loginPacket->status != 3)
            {
                PlatformHelper::showToast("网络链接失败，请稍后再试");
                return;
            }
            
            if (loginPacket->resultIsOK())
            {
                this->unscheduleUpdate();
                ZJHModel::getInstance()->gotoView(VIEW_FORGET_PASSWORD2, json);
            }else
            {
                PlatformHelper::showToast(loginPacket->recvVal["message"].asCString());
            }
        },"user/regValCode",json.toStyledString(),"regValCodeRegist");
    }
}
