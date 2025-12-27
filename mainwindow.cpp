#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QFont>
#include <QItemSelectionModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_historyModel(nullptr)
    , m_proxyModel(nullptr)
    , m_networkManager(nullptr)
{
    ui->setupUi(this);

    // 初始化数据库和历史记录模型
    setupDatabase();
    setupHistoryModel();

    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::handleTranslationFinished);

    // 默认方向：英 -> 中
    if (ui->comboDirection->count() == 0) {
        ui->comboDirection->addItem(tr("英 -> 中"));
        ui->comboDirection->addItem(tr("中 -> 英"));
    }

    ui->textEditResult->setReadOnly(true);
    ui->textEditExamples->setReadOnly(true);

    // 美化：整体字体更柔和一些，适合中文界面
    QFont uiFont(QStringLiteral("Microsoft YaHei UI"));
    uiFont.setPointSize(10);
    this->setFont(uiFont);
    ui->textEditResult->setFont(uiFont);
    ui->textEditExamples->setFont(uiFont);

    ui->tableHistory->setAlternatingRowColors(true);
    ui->tableHistory->horizontalHeader()->setHighlightSections(false);
    ui->tableHistory->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableHistory->verticalHeader()->setVisible(false);
    ui->tableHistory->setShowGrid(false);
    ui->tableHistory->setStyleSheet(
                "QTableView {"
                "  selection-background-color: #409EFF;"
                "  selection-color: white;"
                "}");

    statusBar()->showMessage(tr("就绪"), 2000);
}

MainWindow::~MainWindow()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    delete ui;
}

void MainWindow::setupDatabase()
{
    if (!QSqlDatabase::contains("dict_connection")) {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "dict_connection");
    } else {
        m_db = QSqlDatabase::database("dict_connection");
    }

    m_db.setDatabaseName("dictionary_cache.db");

    if (!m_db.open()) {
        QMessageBox::critical(this, tr("数据库错误"),
                              tr("无法打开数据库：%1").arg(m_db.lastError().text()));
        return;
    }

    QSqlQuery query(m_db);
    const QString createSql =
            "CREATE TABLE IF NOT EXISTS history ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "source TEXT, "
            "target TEXT, "
            "from_lang TEXT, "
            "to_lang TEXT, "
            "example TEXT, "
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ")";

    if (!query.exec(createSql)) {
        QMessageBox::critical(this, tr("数据库错误"),
                              tr("初始化数据表失败：%1").arg(query.lastError().text()));
    }
}

void MainWindow::setupHistoryModel()
{
    if (!m_db.isOpen()) {
        return;
    }

    // 创建源模型（QSqlTableModel）
    m_historyModel = new QSqlTableModel(this, m_db);
    m_historyModel->setTable("history");
    m_historyModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    m_historyModel->setHeaderData(1, Qt::Horizontal, tr("原文"));
    m_historyModel->setHeaderData(2, Qt::Horizontal, tr("译文"));
    m_historyModel->setHeaderData(3, Qt::Horizontal, tr("源语言"));
    m_historyModel->setHeaderData(4, Qt::Horizontal, tr("目标语言"));
    m_historyModel->setHeaderData(5, Qt::Horizontal, tr("例句"));
    m_historyModel->setHeaderData(6, Qt::Horizontal, tr("时间"));

    m_historyModel->select();

    // 创建代理模型（QSortFilterProxyModel）用于过滤和排序
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_historyModel);
    // 设置过滤模式：在所有列中搜索
    m_proxyModel->setFilterKeyColumn(-1); // -1 表示搜索所有列
    // 设置大小写不敏感
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    // 将代理模型绑定到视图
    ui->tableHistory->setModel(m_proxyModel);
    ui->tableHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableHistory->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableHistory->horizontalHeader()->setStretchLastSection(true);
    // 隐藏ID列（注意：在代理模型中，列索引需要映射）
    ui->tableHistory->setColumnHidden(0, true); // 隐藏自增 ID
    
    // 连接搜索框的信号到过滤槽函数
    connect(ui->lineEditFilter, &QLineEdit::textChanged,
            this, &MainWindow::on_lineEditFilter_textChanged);
}

void MainWindow::translateCurrentText()
{
    if (!m_networkManager) {
        QMessageBox::warning(this, tr("错误"), tr("网络模块未初始化"));
        return;
    }

    const QString text = ui->lineEditInput->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请输入要翻译的单词或句子。"));
        return;
    }

    QString fromLang = "en";
    QString toLang = "zh-CN";
    if (ui->comboDirection->currentIndex() == 1) {
        fromLang = "zh-CN";
        toLang = "en";
    }

    // 先从本地缓存中查找
    if (m_db.isOpen()) {
        QSqlQuery query(m_db);
        query.prepare("SELECT target, example FROM history "
                      "WHERE source = ? AND from_lang = ? AND to_lang = ? "
                      "ORDER BY id DESC LIMIT 1");
        query.addBindValue(text);
        query.addBindValue(fromLang);
        query.addBindValue(toLang);
        if (query.exec() && query.next()) {
            const QString cachedTarget = query.value(0).toString();
            const QString cachedExample = query.value(1).toString();

            ui->textEditResult->setPlainText(cachedTarget);
            ui->textEditExamples->setPlainText(cachedExample);
            statusBar()->showMessage(tr("已从本地缓存中获取结果"), 3000);
            return;
        }
    }

    // 走网络请求
    ui->btnTranslate->setEnabled(false);
    statusBar()->showMessage(tr("正在查询中，请稍候..."));

    QUrl url("https://api.mymemory.translated.net/get");
    QUrlQuery query;
    query.addQueryItem("q", text);
    query.addQueryItem("langpair", fromLang + "|" + toLang);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtDictionaryApp/1.0");

    m_networkManager->get(request);
}

void MainWindow::saveHistory(const QString &source, const QString &target,
                             const QString &fromLang, const QString &toLang,
                             const QString &example)
{
    if (!m_db.isOpen()) {
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO history (source, target, from_lang, to_lang, example) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(source);
    query.addBindValue(target);
    query.addBindValue(fromLang);
    query.addBindValue(toLang);
    query.addBindValue(example);

    if (!query.exec()) {
        qWarning("Failed to insert history: %s", qPrintable(query.lastError().text()));
    } else if (m_historyModel) {
        m_historyModel->select();
        // 代理模型会自动同步更新
    }
}

void MainWindow::deleteHistoryByIds(const QList<int> &ids)
{
    if (!m_db.isOpen() || ids.isEmpty()) {
        return;
    }

    QSqlQuery query(m_db);
    m_db.transaction();
    for (int id : ids) {
        query.prepare("DELETE FROM history WHERE id = ?");
        query.addBindValue(id);
        if (!query.exec()) {
            qWarning("Failed to delete history: %s", qPrintable(query.lastError().text()));
        }
    }
    m_db.commit();

    if (m_historyModel) {
        m_historyModel->select();
        // 代理模型会自动同步更新，过滤条件保持不变
    }
}

void MainWindow::on_btnTranslate_clicked()
{
    translateCurrentText();
}

void MainWindow::on_btnClear_clicked()
{
    ui->lineEditInput->clear();
    ui->textEditResult->clear();
    ui->textEditExamples->clear();
    statusBar()->showMessage(tr("已清空"), 2000);
}

void MainWindow::on_lineEditInput_returnPressed()
{
    translateCurrentText();
}

void MainWindow::on_tableHistory_doubleClicked(const QModelIndex &index)
{
    if (!m_historyModel || !m_proxyModel || !index.isValid()) {
        return;
    }

    // 将代理模型的索引映射到源模型
    const QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    const int row = sourceIndex.row();
    const QString source = m_historyModel->data(m_historyModel->index(row, 1)).toString();
    const QString target = m_historyModel->data(m_historyModel->index(row, 2)).toString();
    const QString example = m_historyModel->data(m_historyModel->index(row, 5)).toString();

    ui->lineEditInput->setText(source);
    ui->textEditResult->setPlainText(target);
    ui->textEditExamples->setPlainText(example);
}

void MainWindow::handleTranslationFinished(QNetworkReply *reply)
{
    ui->btnTranslate->setEnabled(true);
    statusBar()->clearMessage();

    if (!reply) {
        return;
    }

    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, tr("网络错误"),
                             tr("请求失败：%1").arg(reply->errorString()));
        return;
    }

    const QByteArray data = reply->readAll();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::warning(this, tr("解析错误"),
                             tr("无法解析服务器返回的数据。"));
        return;
    }

    const QJsonObject root = doc.object();
    const QJsonObject responseData = root.value(QStringLiteral("responseData")).toObject();
    QString translated = responseData.value(QStringLiteral("translatedText")).toString();

    QString exampleText;
    QStringList allTranslations;

    const QJsonArray matches = root.value(QStringLiteral("matches")).toArray();

    for (const QJsonValue &v : matches) {
        const QJsonObject obj = v.toObject();
        const QString t = obj.value(QStringLiteral("translation")).toString();
        if (t.isEmpty()) {
            continue;
        }
        if (allTranslations.contains(t)) {
            continue;
        }
        allTranslations << t;
    }

    // 构造例句（取第一条匹配）
    if (!matches.isEmpty()) {
        const QJsonObject first = matches.first().toObject();
        const QString segment = first.value(QStringLiteral("segment")).toString();
        const QString translation = first.value(QStringLiteral("translation")).toString();
        if (!segment.isEmpty() || !translation.isEmpty()) {
            exampleText = tr("原句：%1\n翻译：%2").arg(segment, translation);
        }
    }

    if (translated.isEmpty()) {
        translated = root.value(QStringLiteral("responseDetails")).toString();
    }

    if (translated.isEmpty() && !allTranslations.isEmpty()) {
        translated = allTranslations.first();
    }

    if (translated.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("未获取到翻译结果。"));
        return;
    }

    // 将多个释义统一格式化输出（不再区分主要/其他，也不展示词性）
    QString displayText;
    const QString header = tr("释义：");
    displayText += header + "\n\n";

    if (!allTranslations.isEmpty()) {
        for (int i = 0; i < allTranslations.size(); ++i) {
            displayText += QStringLiteral("%1. %2\n")
                               .arg(i + 1)
                               .arg(allTranslations.at(i));
        }
    } else {
        // 没有 matches 时，至少把 translated 显示出来
        displayText += QStringLiteral("1. %1\n").arg(translated);
    }

    ui->textEditResult->setPlainText(displayText.trimmed());
    ui->textEditExamples->setPlainText(exampleText);

    // 保存到历史
    QString fromLang = "en";
    QString toLang = "zh-CN";
    if (ui->comboDirection->currentIndex() == 1) {
        fromLang = "zh-CN";
        toLang = "en";
    }

    const QString sourceText = ui->lineEditInput->text().trimmed();
    // 将展示给用户的多义结果整体保存下来，方便下次直接从缓存恢复
    saveHistory(sourceText, displayText.trimmed(), fromLang, toLang, exampleText);
}

void MainWindow::on_btnDeleteSelected_clicked()
{
    if (!m_historyModel || !m_proxyModel) {
        return;
    }

    QItemSelectionModel *selModel = ui->tableHistory->selectionModel();
    if (!selModel) {
        return;
    }

    const QModelIndexList rows = selModel->selectedRows();
    if (rows.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请先选择要删除的历史记录。"));
        return;
    }

    QList<int> ids;
    ids.reserve(rows.size());
    for (const QModelIndex &proxyIndex : rows) {
        // 将代理模型的索引映射到源模型
        const QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        const int row = sourceIndex.row();
        const int id = m_historyModel->data(m_historyModel->index(row, 0)).toInt();
        if (id > 0) {
            ids << id;
        }
    }

    if (ids.isEmpty()) {
        return;
    }

    if (QMessageBox::question(this, tr("确认删除"),
                              tr("确定要删除选中的 %1 条记录吗？").arg(ids.size()))
        == QMessageBox::Yes) {
        deleteHistoryByIds(ids);
    }
}

void MainWindow::on_btnClearHistory_clicked()
{
    if (!m_db.isOpen() || !m_historyModel) {
        return;
    }

    if (QMessageBox::question(this, tr("清空历史"),
                              tr("此操作将删除所有查询历史，是否继续？"))
        != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query(m_db);
    if (!query.exec("DELETE FROM history")) {
        QMessageBox::warning(this, tr("错误"),
                             tr("清空历史失败：%1").arg(query.lastError().text()));
        return;
    }

    m_historyModel->select();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,
                       tr("关于"),
                       tr("2023414290317 刘威"));
}

void MainWindow::on_lineEditFilter_textChanged(const QString &text)
{
    if (!m_proxyModel) {
        return;
    }
    // 使用正则表达式进行过滤（支持部分匹配）
    m_proxyModel->setFilterRegularExpression(QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));
}
