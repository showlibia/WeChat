#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QRandomGenerator>
#include <QTimer>
// #include <chrono>
// #include <thread>
#include "chatuserwid.h"
#include "loadingdlg.h"

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog), _mode(ChatUIMode::ChatMode), _state(ChatUIMode::ChatMode), _b_loading(false)
{
    ui->setupUi(this);
    ui->search_btn->SetState("normal", "hover", "press");
    ui->search_edit->setMaxLength(15);

    QAction* search_action = new QAction(ui->search_edit);
    search_action->setIcon(QIcon(":/res/search.png"));
    ui->search_edit->addAction(search_action, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));

    ui->search_edit->setClearButtonEnabled(true);

    ShowSearch(false);

    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this, &ChatDialog::slot_loading_chat_user);

    addChatUserList();

    connect(ui->search_edit, &QLineEdit::textChanged, this, [this](const QString &text){
        if(text.isEmpty()) {
            ShowSearch(false);
        }
    });

    QPixmap pixmap(":/res/head_1.jpg");
    ui->side_head_lb->setPixmap(pixmap); // 将图片设置到QLabel上
    QPixmap scaledPixmap = pixmap.scaled( ui->side_head_lb->size(), Qt::KeepAspectRatio); // 将图片缩放到label的大小
    ui->side_head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    ui->side_head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

    ui->side_chat_lb->setProperty("state","normal");

    ui->side_chat_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    ui->side_contact_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    AddLBGroup(ui->side_chat_lb);
    AddLBGroup(ui->side_contact_lb);

    connect(ui->side_chat_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);
    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_text_changed);

    //检测鼠标点击位置判断是否要清空搜索框
    this->installEventFilter(this); // 安装事件过滤器

    //设置聊天label选中状态
    ui->side_chat_lb->SetSelected(true);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::ShowSearch(bool search)
{
    if(search) {
        ui->chat_user_list->hide();
        ui->search_list->show();
        ui->contact_list->hide();
        _mode = ChatUIMode::SearchMode;
    } else if(_state == ChatUIMode::ChatMode) {
        ui->chat_user_list->show();
        ui->search_list->hide();
        ui->contact_list->hide();
        _mode = ChatUIMode::ChatMode;
    } else if(_state == ChatUIMode::ContactMode) {
        ui->chat_user_list->hide();
        ui->search_list->hide();
        ui->contact_list->show();
        _mode = ChatUIMode::ContactMode;
    }
}

void ChatDialog::addChatUserList()
{
    for(int i = 0; i < 13; ++i) {
        int random = QRandomGenerator::global()->bounded(100);
        int str = random % strs.size();
        int name = random % names.size();
        int head = random % heads.size();

        auto chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name], heads[head], strs[str]);
        auto item = new QListWidgetItem;
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        // 将item设置为
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        handleGlobalMousePress(mouseEvent);
    }
    return QDialog::eventFilter(watched, event);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *event)
{
    // 实现点击位置的判断和处理逻辑
    // 先判断是否处于搜索模式，如果不处于搜索模式则直接返回
    if( _mode != ChatUIMode::SearchMode){
        return;
    }

    // 将鼠标点击位置转换为搜索列表坐标系中的位置
    QPoint posInSearchList = ui->search_list->mapFromGlobal(event->globalPos());
    // 判断点击位置是否在聊天列表的范围内
    if (!ui->search_list->rect().contains(posInSearchList)) {
        // 如果不在聊天列表内，清空输入框
        ui->search_edit->clear();
        ShowSearch(false);
    }
}

void ChatDialog::CloseFindDlg()
{

}

void ChatDialog::slot_loading_chat_user()
{
    if(_b_loading) return;
    _b_loading = true;

    // 创建加载项
    LoadingDlg *loadingDlg = new LoadingDlg(this);
    QListWidgetItem* item = new QListWidgetItem;

    loadingDlg->setFixedSize(ui->chat_user_list->width(), 30);
    item->setSizeHint(loadingDlg->size());

    ui->chat_user_list->addItem(item);
    ui->chat_user_list->setItemWidget(item, loadingDlg);

    QCoreApplication::processEvents();

    QTimer::singleShot(0, this, [this, item]() {
        addChatUserList();

        if(int row = ui->chat_user_list->row(item); row != -1) {
            ui->chat_user_list->takeItem(row);
            delete item;
        }
        _b_loading = false;
    });
}

void ChatDialog::AddLBGroup(StateWidget *lb)
{
    _lb_list.push_back(lb);
}

void ChatDialog::slot_side_chat()
{
    qDebug()<< "receive side chat clicked";
    ClearLabelState(ui->side_chat_lb);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::ChatMode;
    ShowSearch(false);
}

void ChatDialog::slot_side_contact()
{
    qDebug()<< "receive side contact clicked";
    ClearLabelState(ui->side_contact_lb);
    //设置
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    _state = ChatUIMode::ContactMode;
    ShowSearch(false);
}

void ChatDialog::slot_text_changed(const QString &str)
{
    if (!str.isEmpty()) {
        ShowSearch(true);
    }
}

void ChatDialog::ClearLabelState(StateWidget *lb)
{
    for(auto & ele: _lb_list){
        if(ele == lb){
            continue;
        }

        ele->ClearState();
    }
}
