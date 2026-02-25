#include "User.h"
#include "ui_User.h"
#include "ui_Users.h"

#include <QTemporaryFile>

#include "MockUserRoleManager.h"

User::User(IUserRoleManager* roleManager, bool editing, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::User),
    _roleManager(roleManager)
{
    ui->setupUi(this);

    this->setModal(true);

    this->setAttribute(Qt::WA_DeleteOnClose);

    if (editing) {
        this->setWindowTitle("Редактирование пользователя");
        this->ui->pb_save->setText("Сохранить");

        this->ui->le_name->setEnabled(false);
        this->ui->cb_level->setEnabled(false);
    } else {
        this->setWindowTitle("Добавление пользователя");
        this->ui->pb_save->setText("Добавить");

        this->ui->le_name->setEnabled(true);
        this->ui->cb_level->setEnabled(true);
    }

    if (_roleManager) {
        QStringList roles = editing ? _roleManager->getRolesForEditing() : _roleManager->getRolesForAdding();
        ui->cb_level->addItems(roles);
        if (!roles.isEmpty())
            ui->cb_level->setCurrentIndex(0);
    }

    connect(this->ui->pb_save, &QPushButton::clicked, this, &User::on_save);
    connect(this->ui->pb_cancel, &QPushButton::clicked, this, &User::close);
}

User::~User() {
    delete ui;
}

void User::set_user(const std::string &name, const std::string &level) {
    this->ui->le_name->setText(name.c_str());
    this->ui->cb_level->setCurrentText(level.c_str());
}

void User::on_save() {
    std::string name = this->ui->le_name->text().toStdString();
    std::string level = this->ui->cb_level->currentText().toStdString();
    std::string password = this->ui->le_password->text().toStdString();

    if (name.empty() || level.empty() || password.empty()) {
        AQP::warning(this, tr("Внимание"), tr("Ошибка"), tr("Неверно заданы параметры пользователя"), 0, 10);
    } else {
        emit save_user(name.c_str(), level.c_str(), password.c_str());
        this->accept();
    }
}

/*********************************/

Users::Users(IUserRoleManager* roleManager,
            const std::string& filename,
            QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Users),
    _filename(filename),
    _roleManager(roleManager)
{
    ui->setupUi(this);

    this->setWindowTitle("Редактирование пользователей");
    this->setModal(true);

    //this->setAttribute(Qt::WA_DeleteOnClose);

    this->ui->lw_users->setSelectionMode(QAbstractItemView::SingleSelection);

    this->load();

    //connect(this->ui->pb_add, &QPushButton::clicked, this, &Users::on_add);
    connect(this->ui->pb_save, &QPushButton::clicked, this, &Users::save);
    connect(this->ui->pb_cancel, &QPushButton::clicked, this, &Users::close);

    connect(this->ui->pb_add, &QPushButton::clicked, this, &Users::_add);
    connect(this->ui->pb_delete, &QPushButton::clicked, this, &Users::_delete);

    connect(this->ui->lw_users, &QListWidget::doubleClicked, this, &Users::double_clicked);
}

Users::~Users() {
    delete ui;
}

void Users::show() {
    if (_roleManager) {
        QString currentRole = QString::fromStdString(auth_user.user.level);
        bool canAdd = _roleManager->canAddUser(currentRole);
        ui->pb_add->setEnabled(canAdd);
        ui->pb_delete->setEnabled(canAdd);  // можно сделать отдельную проверку canDeleteUser
    }
    QDialog::show();
}

void Users::_save() {
    this->save();
    this->accept();
}

void Users::_add() {
    _adding = true;
    _editing = false;
    User *user = new User(_roleManager, false, this);
    connect(user, &User::save_user, this, &Users::save_user);
    user->show();
}

void Users::_delete() {
    if (_roleManager) {
        QString currentRole = QString::fromStdString(auth_user.user.level);
        // Получаем роль выбранного пользователя
        QList<QListWidgetItem*> selected = ui->lw_users->selectedItems();
        if (selected.size() == 1) {
            User_s user_s;
            user_s.from_text(selected.at(0)->text());
            QString targetRole = QString::fromStdString(user_s.level);
            if (!_roleManager->canDeleteUser(currentRole, targetRole)) {
                AQP::warning(this, tr("Внимание"), tr("Ошибка"), tr("Удаление пользователя недоступно"), 0, 10);
                return;
            }
        }
    }

    QList<QListWidgetItem*> selected = this->ui->lw_users->selectedItems();
    if (selected.size() == 1) {
        QListWidgetItem* item = selected.at(0);
        User_s user_s;
        user_s.from_text(item->text());

        this->deleteUser(user_s.name, user_s.level, user_s.password);
    }
}

void Users::double_clicked(const QModelIndex& index) {
    _adding = false;
    _editing = true;

    if(!index.isValid()) {
        return;
    }

    if (_roleManager) {
        QString currentRole = QString::fromStdString(auth_user.user.level);
        if(QListWidgetItem* item = static_cast<QListWidget*>(sender())->item(index.row())) {
            User_s user_s;
            user_s.from_text(item->text());
            QString targetRole = QString::fromStdString(user_s.level);
            if (!_roleManager->canEditUser(currentRole, targetRole)) {
                AQP::warning(this, tr("Внимание"), tr("Ошибка"), tr("Редактирование недоступно"), 0, 10);
                return;
            }
            // открываем диалог редактирования
            _editing = true;
            User *user = new User(_roleManager, true, this);
            user->set_user(user_s.name, user_s.level);
            connect(user, &User::save_user, this, &Users::save_user);
            user->show();
        }
    }
}

void Users::save_user(const char *name, const char *level, const char *password) {
    bool match = false;

    if (_adding) {
        std::string name_s(name);
        for (auto &user : _users) {
            std::string user_name_s(user.name);
            if (name_s == user_name_s) {
                //qCDebug(logDebug()) << "user already exist";
                AQP::warning(this, tr("Внимание"), tr("Ошибка"), tr("Пользователь с указанным именем уже существует"), 0, 10);
                match = true;
                break;
            }
        }

        if (!match) {
            this->addUser(name, level, password);
            this->update_lw();
        }
    }

    if (_editing) {
        std::string name_s(name);
        std::string level_s(level);
        for (size_t j = 0; j < _users.size(); ++j) {
            auto user = _users.at(j);
            std::string user_name_s(user.name);
            std::string user_level_s(user.level);
            if (name_s == user_name_s) {
                if (level_s == user_level_s) {
                    std::string password_hash = this->hash(QString(password));
                    //qCDebug(logDebug) << password_hash;

                    for (int k = 0; k < UserCharLength; ++k) {
                        _users[j].password[k] = '\0';
                    }

                    int i = 0;
                    for (auto &c : password_hash) {
                        if (i < UserCharLength) {
                            _users[j].password[i] = c;
                        }
                        ++i;
                    }
                    //qCDebug(logDebug()) << "password updated";
                    break;
                }
            }
        }
    }

    _adding = false;
    _editing = false;
}

void Users::load() {
    std::ifstream ifst(_filename, std::ios::binary);

    if(!ifst) {
        ifst.open(_filename, std::ios::out);
        ifst.close(); // TODO: copy paste
        ifst.open(_filename, std::ios::binary);
    }

    ifst.seekg(0, std::ios::end);
    int tellg = ifst.tellg();
    if (tellg != -1) {
        int n = tellg / sizeof(User_s);

        ifst.seekg(0, std::ios::beg);
        for (int i = 0; i < n; ++i) {
            _users.push_back(read_user(ifst));
        }
    }

    if (_filename == UserDefaultFilename) {
        if (_users.empty()) {
            std::string password = "123";
            this->addUser("acs", "Разработчик", "5041511"); //developer
            this->addUser("admin", "Администратор", password.c_str()); //superuser
            this->addUser("Пользователь", "Пользователь", "Пользователь");
            this->save();
        }
        User_s *default_user = this->find("Пользователь", "Пользователь");
        if (default_user == nullptr) {
            this->addUser("Пользователь", "Пользователь", "Пользователь");
        }
    }

    this->update_lw();
}

void Users::addUser(const char* name, const char* level, const char* password) {
    User_s user;
    strncpy(user.name, name, UserCharLength);
    strncpy(user.level, level, UserCharLength);

    std::string password_hash = this->hash(QString(password));
    //qCDebug(logDebug) << password_hash;

    int i = 0;
    for (auto &c : password_hash) {
        if (i < UserCharLength) {
            user.password[i] = c;
        }
        ++i;
    }

    _users.push_back(user);
}

void Users::deleteUser(const char* name, const char* level, const char* password) {
    Q_UNUSED(password);

    int index_to_delete = -1;
    int i = 0;
    std::string name_s(name);
    std::string level_s(level);
    for (auto &user : _users) {
        std::string user_name_s(user.name);
        std::string user_level_s(user.level);
        if (name_s == user_name_s) {
            if (level_s == user_level_s) {
                index_to_delete = i;
                break;
            }
        }
        ++i;
    }

    if ((index_to_delete != -1) && ((size_t)index_to_delete < _users.size())) {
        //qCDebug(logDebug()) << "accept deleting";
        _users.erase(_users.begin() + index_to_delete);
    }

    this->update_lw();
}

User_s Users::read_user(std::ifstream &ifst) {
    User_s user;
    ifst.read((char*)&user, sizeof(User_s));
    return user;
}

User_s* Users::find(const std::string& name) {
    for (auto& user : _users) { // TODO: find_if
        if (name == user.name)
            return &user;
    }
    return nullptr;
}

User_s* Users::find(const std::string& name, const std::string& password) {
    std::string password_hash = this->hash(password.c_str());

    for (auto& user : _users) {
        int size = 0;
        for (int i = UserCharLength - 1; i >= 0; --i) {
            if (user.password[i] != '\0') {
                size = i + 1;
                break;
            }
        }

//        QByteArray ba(user.password, size);
//        QString user_password_hash(ba, size);
        std::string user_password_hash;
        for(int i = 0; i < size; ++i) {
            user_password_hash.push_back(user.password[i]);
        }
        if ((name == user.name) && (password_hash == user_password_hash)) {
            return &user;
        } else if ((name == user.name) && (password_hash != user_password_hash)) {
            //wrong password
            emit password_fail();
            return nullptr;
            //qCDebug(logDebug()) << password_hash;
            //qCDebug(logDebug()) << user_password_hash;
            //qCDebug(logDebug()) << password_hash.size() << user_password_hash.size();
        }
    }
    emit user_fail();
    return nullptr;
}

void Users::save() {
    std::ofstream ofst;
    ofst.open(_filename, std::ios::out);

    for (auto& user : _users) {
        ofst.write((char*)&user, sizeof(User_s));
    }
}

std::string Users::hash(QString s) {
    QByteArray ba = s.toUtf8();
    QString password_hash = QCryptographicHash::hash(ba, QCryptographicHash::Sha256);
    password_hash.replace("\n", "").replace("\r", "");
    return password_hash.toStdString();
}

QStringList Users::get_list() const {
    QStringList sl;

    for (const auto &user : _users) {
        sl << user.get_text().c_str();
    }

    return sl;
}

void Users::update_lw(){
    QStringList sl = this->get_list();
    this->ui->lw_users->clear();
    this->ui->lw_users->addItems(sl);
}

// Тест добавления и поиска пользователя
void testUsersAddFind() {
    MockUserRoleManager mock;
    QTemporaryFile tempFile;
    tempFile.open(); // создаёт уникальное имя
    Users users(&mock, tempFile.fileName().toStdString());

    users.addUser("testuser", "Администратор", "pass123");
    User_s* found = users.find("testuser");
    ASSERT(found != nullptr);
    ASSERT_EQUAL(std::string(found->name), "testuser");
    ASSERT_EQUAL(std::string(found->level), "Администратор");

    User_s* foundWithPass = users.find("testuser", "pass123");
    ASSERT(foundWithPass != nullptr);

    User_s* wrongPass = users.find("testuser", "wrong");
    ASSERT(wrongPass == nullptr); // неверный пароль
}

// Тест получения списка пользователей
void testUsersGetList() {
    MockUserRoleManager mock;
    QTemporaryFile tempFile;
    tempFile.open(); // создаёт уникальное имя
    Users users(&mock, tempFile.fileName().toStdString());

    users.addUser("acs", "Разработчик", "passacs");
    users.addUser("user1", "Администратор", "p1");
    users.addUser("user2", "Пользователь", "p2");

    QStringList list = users.get_list();
    ASSERT_EQUAL(list.size(), 3);
    ASSERT(list.contains("Разработчик: acs"));
    ASSERT(list.contains("Администратор: user1"));
    ASSERT(list.contains("Пользователь: user2"));
}

// Тест загрузки и сохранения (с временным файлом)
void testUsersLoadSave() {
    MockUserRoleManager mock;
    QTemporaryFile tempFile;
    tempFile.open(); // создаёт уникальное имя
    Users users(&mock, tempFile.fileName().toStdString());

    // Сохраняем тестовых пользователей
    users.addUser("acs", "Разработчик", "passacs");
    users.addUser("alice", "Администратор", "passA");
    users.addUser("bob", "Пользователь", "passB");
    users.save();

    // Создаём новый объект Users и загружаем данные
    Users users2(&mock, tempFile.fileName().toStdString());

    QStringList list = users2.get_list();
    ASSERT_EQUAL(list.size(), 3);
    ASSERT(list.contains("Разработчик: acs"));
    ASSERT(list.contains("Администратор: alice"));
    ASSERT(list.contains("Пользователь: bob"));

    // Проверяем поиск
    User_s* alice = users2.find("alice", "passA");
    ASSERT(alice != nullptr);
    ASSERT_EQUAL(std::string(alice->name), "alice");
}

// Тест удаления пользователя (только логика удаления из вектора)
void testUsersDeleteLogic() {
    MockUserRoleManager mock;
    QTemporaryFile tempFile;
    tempFile.open(); // создаёт уникальное имя
    Users users(&mock, tempFile.fileName().toStdString());

    users.addUser("acs", "Разработчик", "passacs");
    users.addUser("alice", "Администратор", "passA");
    users.addUser("bob", "Пользователь", "passB");

    QStringList list = users.get_list();
    ASSERT_EQUAL(list.size(), 3);
    ASSERT(list.contains("Разработчик: acs"));
    ASSERT(list.contains("Администратор: alice"));
    ASSERT(list.contains("Пользователь: bob"));

    users.deleteUser("bob", "Пользователь", "passB");

    list = users.get_list();
    ASSERT_EQUAL(list.size(), 2);
    ASSERT(list.contains("Разработчик: acs"));
    ASSERT(list.contains("Администратор: alice"));
    ASSERT(!list.contains("Пользователь: bob"));

    users.deleteUser("alice", "Администратор", "passA");

    list = users.get_list();
    ASSERT_EQUAL(list.size(), 1);
    ASSERT(list.contains("Разработчик: acs"));
    ASSERT(!list.contains("Администратор: alice"));
    ASSERT(!list.contains("Пользователь: bob"));
}

// Тест проверки прав через менеджер (метод show)
void testUsersShowPermissions() {
    MockUserRoleManager mock;
    QTemporaryFile tempFile;
    tempFile.open(); // создаёт уникальное имя
    Users users(&mock, tempFile.fileName().toStdString());

    users.addUser("acs", "Разработчик", "passacs");
    users.addUser("user1", "Администратор", "p1");
    users.addUser("user2", "Пользователь", "p2");

    QStringList list = users.get_list();
    ASSERT_EQUAL(list.size(), 3);
    ASSERT(list.contains("Разработчик: acs"));
    ASSERT(list.contains("Администратор: user1"));
    ASSERT(list.contains("Пользователь: user2"));

    // Устанавливаем текущего пользователя в синглтон
    User_s current;
    strncpy(current.level, "Разработчик", UserCharLength);
    auth_user.user = current;
    auth_user.setRoleManager(&mock);

    bool canAdd = mock.canAddUser("Разработчик");
    ASSERT(canAdd); // Разработчик может добавлять

    canAdd = mock.canAddUser("Администратор");
    ASSERT(canAdd); // Администратор может добавлять

    canAdd = mock.canAddUser("Пользователь");
    ASSERT(!canAdd); // Пользователь не может добавлять
}

void UserTest() {
    testUsersAddFind();
    testUsersGetList();
    testUsersLoadSave();
    testUsersDeleteLogic();
    testUsersShowPermissions();
}
