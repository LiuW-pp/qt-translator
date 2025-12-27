# 2.4 Model/View 设计说明

## 一、使用的 Model 类型

本系统使用了 **QSqlTableModel**（Qt SQL模块提供的标准模型类），用于将 SQLite 数据库中的 `history` 表数据绑定到 `QTableView` 视图进行展示。

**代码实现：**
```cpp
m_historyModel = new QSqlTableModel(this, m_db);
m_historyModel->setTable("history");
m_historyModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
m_historyModel->select();
ui->tableHistory->setModel(m_historyModel);
```

**说明：** QSqlTableModel 是 Qt 提供的标准数据库模型类，专门用于将数据库表映射到 Qt 的 Model/View 架构中。它继承自 `QAbstractTableModel`，自动处理数据库查询、数据缓存和视图更新，无需自定义 Model 类即可实现数据展示功能。

---

## 二、为什么使用 Model/View 而不是直接往控件塞数据？

### 1. **数据与视图分离，降低耦合度**
直接往控件（如 QTableWidget）塞数据需要手动管理每个单元格的内容，数据变化时需要手动更新所有相关控件。而 Model/View 架构将数据（Model）和展示（View）完全分离，数据变化时只需调用 `model->select()`，视图会自动同步更新，代码更简洁、维护性更好。

### 2. **支持多视图共享同一数据源**
一个 Model 可以同时绑定到多个 View（如 QTableView、QListView、QTreeView），数据只需维护一份，多个视图会自动同步。如果直接使用 QTableWidget 等控件，需要手动维护多份数据副本，容易造成数据不一致。

### 3. **更高效的数据处理**
QSqlTableModel 支持延迟加载和分页查询，对于大量数据不会一次性加载到内存，而是按需从数据库读取。而直接塞数据的方式需要一次性将所有数据加载到内存，内存占用大、响应慢。

### 4. **符合 Qt 的设计理念**
Qt 的 Model/View 架构是业界成熟的 MVC 模式实现，提供了标准化接口，便于扩展和维护。直接操作控件虽然简单，但在复杂场景下会面临性能和可维护性的问题。

---

## 三、使用 ProxyModel 如何过滤数据？

**说明：** 本系统实际使用了 `QSortFilterProxyModel` 实现数据的动态过滤功能，用户在搜索框中输入关键字时，历史记录表格会实时显示匹配的结果。

### ProxyModel 的工作原理：

ProxyModel（代理模型）是 Model/View 架构中的中间层，位于原始 Model 和 View 之间。它接收原始 Model 的数据，经过过滤、排序等处理后，再传递给 View 显示。这种设计保持了原始数据源的完整性，同时提供了灵活的视图展示能力。

### 代码实现：

```cpp
// 创建源模型（QSqlTableModel）
m_historyModel = new QSqlTableModel(this, m_db);
m_historyModel->setTable("history");
m_historyModel->select();

// 创建代理模型（QSortFilterProxyModel）用于过滤和排序
m_proxyModel = new QSortFilterProxyModel(this);
m_proxyModel->setSourceModel(m_historyModel);
// 设置过滤模式：在所有列中搜索（-1表示搜索所有列）
m_proxyModel->setFilterKeyColumn(-1);
// 设置大小写不敏感
m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

// 将代理模型绑定到视图（而不是直接绑定源模型）
ui->tableHistory->setModel(m_proxyModel);

// 搜索框文本变化时触发过滤
void MainWindow::on_lineEditFilter_textChanged(const QString &text)
{
    // 使用正则表达式进行过滤（支持部分匹配）
    m_proxyModel->setFilterRegularExpression(
        QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));
}
```

### 关键特性：

1. **索引映射**：当用户在代理模型的视图上进行操作（如双击、选择）时，需要将代理模型的索引映射回源模型。使用 `mapToSource()` 方法实现索引转换：
   ```cpp
   const QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
   ```

2. **动态过滤**：用户在搜索框输入关键字时，通过信号槽机制实时调用 `setFilterRegularExpression()` 更新过滤条件，视图会自动刷新显示匹配的记录。

3. **全列搜索**：设置 `setFilterKeyColumn(-1)` 表示在所有列中搜索关键字，用户可以通过输入原文、译文等内容快速定位历史记录。

**优势：** ProxyModel 可以实现动态过滤，用户输入关键字时实时更新过滤结果，无需修改原始 Model 的数据，保持了数据源的完整性。此外，ProxyModel 还支持列排序、列重排等功能，为视图展示提供了更大的灵活性。

---

## 四、View（QTableView）如何绑定 Model？

### 绑定过程（代码实现）：

本系统采用**两层模型架构**：源模型（QSqlTableModel）+ 代理模型（QSortFilterProxyModel），然后将代理模型绑定到视图。

```cpp
// 步骤1：创建源 Model 并配置
m_historyModel = new QSqlTableModel(this, m_db);
m_historyModel->setTable("history");
m_historyModel->select(); // 从数据库加载数据

// 步骤2：设置表头显示（可选）
m_historyModel->setHeaderData(1, Qt::Horizontal, tr("原文"));
m_historyModel->setHeaderData(2, Qt::Horizontal, tr("译文"));
// ... 其他列

// 步骤3：创建代理 Model 并设置源模型
m_proxyModel = new QSortFilterProxyModel(this);
m_proxyModel->setSourceModel(m_historyModel);

// 步骤4：将代理 Model 绑定到 View（而不是直接绑定源模型）
ui->tableHistory->setModel(m_proxyModel);

// 步骤5：配置 View 的显示属性（可选）
ui->tableHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
ui->tableHistory->setColumnHidden(0, true); // 隐藏ID列
```

### 绑定机制说明：

1. **setModel() 方法**：QTableView 的 `setModel()` 方法会建立 View 与 Model 之间的连接。View 会监听 Model 的 `dataChanged()`、`rowsInserted()` 等信号，当 Model 数据变化时自动更新显示。在本系统中，View 绑定的是代理模型（m_proxyModel），而不是直接绑定源模型（m_historyModel）。

2. **数据同步**：当调用 `m_historyModel->select()` 刷新数据时，源模型的数据变化会通过代理模型传递给 View，View 会自动收到通知并重新渲染表格，无需手动更新。代理模型会自动处理数据变化，保持过滤条件不变。

3. **索引映射**：由于使用了代理模型，当需要从 View 中获取源模型的数据时，必须使用 `mapToSource()` 方法将代理模型的索引映射回源模型的索引。例如在删除和双击操作中：
   ```cpp
   const QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
   const int row = sourceIndex.row();
   ```

4. **双向绑定**：虽然本系统中 View 设置为只读（`setEditTriggers(NoEditTriggers)`），但 Model/View 架构支持双向数据流，View 的编辑可以通过 Model 直接反映到数据库。

---

## 五、Model/View 架构示意图





这种架构实现了数据、视图和控制的完全分离，通过代理模型的引入进一步增强了数据展示的灵活性（过滤、排序），提高了代码的可维护性和可扩展性。

