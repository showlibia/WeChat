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
