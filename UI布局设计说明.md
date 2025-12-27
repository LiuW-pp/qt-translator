# 2.7 UI 布局设计

## 一、主界面布局结构

本系统采用**分层布局结构**，整体使用垂直布局（QVBoxLayout），内部使用水平布局（QHBoxLayout）和分割器（QSplitter）组织各个功能区域。

**布局层次结构：**
```
MainWindow (QMainWindow)
└── centralwidget (QWidget)
    └── verticalLayout (QVBoxLayout) - 主垂直布局
        ├── topWidget (QWidget) - 顶部输入区域
        │   └── horizontalLayout (QHBoxLayout) - 水平布局
        │       ├── labelInput (QLabel)
        │       ├── lineEditInput (QLineEdit)
        │       ├── labelDirection (QLabel)
        │       ├── comboDirection (QComboBox)
        │       ├── btnTranslate (QPushButton)
        │       └── btnClear (QPushButton)
        │
        ├── splitter (QSplitter) - 水平分割器
        │   ├── groupBoxResult (QGroupBox) - 左侧翻译结果
        │   │   └── verticalLayoutResult (QVBoxLayout)
        │   │       └── textEditResult (QTextEdit)
        │   │
        │   └── groupBoxHistory (QGroupBox) - 右侧历史记录
        │       └── verticalLayoutHistory (QVBoxLayout)
        │           ├── horizontalLayoutFilter (QHBoxLayout) - 搜索栏
        │           │   ├── labelFilter (QLabel)
        │           │   └── lineEditFilter (QLineEdit)
        │           ├── tableHistory (QTableView)
        │           └── horizontalLayoutHistoryButtons (QHBoxLayout) - 按钮区
        │               ├── horizontalSpacerHistory (QSpacerItem)
        │               ├── btnDeleteSelected (QPushButton)
        │               └── btnClearHistory (QPushButton)
        │
        └── groupBoxExamples (QGroupBox) - 底部例句区域
            └── verticalLayoutExamples (QVBoxLayout)
                └── textEditExamples (QTextEdit)
```

**UI 截图要求：**
1. **主界面整体截图**：展示完整的程序界面，包括顶部输入栏、中间的翻译结果和历史记录区域、底部的例句区域
2. **翻译功能截图**：输入单词（如"hello"），点击翻译按钮后，显示翻译结果和例句的效果
3. **历史记录功能截图**：展示历史记录表格中有多条记录的情况，以及搜索过滤功能的使用效果
4. **删除功能截图**：选中多条历史记录后，点击删除按钮并显示确认对话框的界面

---

## 二、使用的布局管理器

### 1. **QVBoxLayout（垂直布局）**
- **用途**：主窗口的整体垂直排列、翻译结果区域、历史记录区域、例句区域的内部布局
- **特点**：控件按从上到下的顺序排列，自动调整宽度以适应容器

### 2. **QHBoxLayout（水平布局）**
- **用途**：顶部输入栏（输入框、下拉框、按钮水平排列）、历史记录搜索栏、历史记录按钮区
- **特点**：控件按从左到右的顺序排列，自动调整高度以适应容器

### 3. **QSplitter（分割器）**
- **用途**：分隔翻译结果区域和历史记录区域
- **特点**：用户可以拖拽分割线调整左右两个区域的宽度比例，提供灵活的界面布局

**布局管理器使用统计：**
- QVBoxLayout：4 个（主布局 + 3 个 GroupBox 内部布局）
- QHBoxLayout：3 个（顶部输入区 + 搜索栏 + 按钮区）
- QSplitter：1 个（中间区域分割）

---

## 三、UI 样式与功能对应关系

### 1. **顶部输入区域**
- **样式**：水平布局，标签 + 输入框 + 下拉框 + 按钮
- **功能**：
  - `lineEditInput`：用户输入待翻译文本，支持回车键触发翻译
  - `comboDirection`：选择翻译方向（英→中 / 中→英）
  - `btnTranslate`：点击后触发 `translateCurrentText()` 执行翻译
  - `btnClear`：清空输入框和结果显示区域

### 2. **翻译结果区域（左侧）**
- **样式**：GroupBox 标题 + QTextEdit 只读文本框
- **功能**：显示翻译结果的多条释义（编号列表格式），只读模式防止用户误编辑

### 3. **历史记录区域（右侧）**
- **样式**：GroupBox + 搜索栏 + 表格 + 按钮区
- **功能**：
  - `lineEditFilter`：实时过滤历史记录（通过 ProxyModel 实现）
  - `tableHistory`：显示历史记录表格（Model/View 架构）
  - `btnDeleteSelected`：删除选中的历史记录（支持多选）
  - `btnClearHistory`：清空所有历史记录

### 4. **例句区域（底部）**
- **样式**：GroupBox + QTextEdit 只读文本框
- **功能**：显示翻译结果对应的例句（原句 + 翻译），只读模式

### 5. **按钮样式与交互**
- **翻译按钮**：发送网络请求时自动禁用（`setEnabled(false)`），防止重复请求
- **删除按钮**：点击后弹出确认对话框，确认后执行批量删除操作
- **表格选择**：支持多行选择（ExtendedSelection），选中行高亮显示（蓝色背景）

**样式表应用：**
```cpp
// 历史记录表格样式
ui->tableHistory->setStyleSheet(
    "QTableView {"
    "  selection-background-color: #409EFF;"
    "  selection-color: white;"
    "}");
```

---

## 四、UI 设计逻辑说明

### 设计原则：
1. **功能分区清晰**：顶部输入、中间结果展示、底部例句补充，层次分明
2. **左右对比布局**：翻译结果和历史记录并排显示，方便用户对比和回顾
3. **响应式布局**：使用 Splitter 允许用户调整布局比例，适应不同使用习惯
4. **操作流程顺畅**：输入 → 翻译 → 查看结果 → 管理历史，符合用户使用习惯
5. **状态反馈明确**：按钮禁用、状态栏提示、确认对话框，确保用户了解操作状态

### 布局优势：
- **空间利用高效**：Splitter 分隔器让用户可以自由调整左右区域大小
- **信息层次清晰**：主次信息通过区域大小和位置区分（结果区较大，例句区较小）
- **操作便捷**：常用功能（翻译、清空）在顶部，管理功能（删除、筛选）在侧边

