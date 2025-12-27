# GitHub 上传指南

## 前置准备

1. **注册GitHub账号**（如果还没有）
   - 访问 https://github.com
   - 注册新账号

2. **安装Git**（如果还没有安装）
   - 下载：https://git-scm.com/download/win
   - 安装时选择默认选项即可

3. **配置Git用户信息**（如果还没有配置）
   ```bash
   git config --global user.name "你的名字"
   git config --global user.email "your.email@example.com"
   ```

## 步骤一：在GitHub创建仓库

1. 登录GitHub
2. 点击右上角的 **"+"** 按钮，选择 **"New repository"**
3. 填写仓库信息：
   - **Repository name**: `qt-translator` 或 `translator-app`（建议使用英文名称）
   - **Description**: `Qt-based translation tool with SQLite cache`（可选）
   - **Visibility**: 选择 **Public**（公开）或 **Private**（私有）
   - **不要勾选** "Initialize this repository with a README"（我们已经有了README.md）
   - **不要添加** .gitignore 或 license（我们已经有了.gitignore）
4. 点击 **"Create repository"** 按钮

## 步骤二：连接本地仓库到GitHub

创建仓库后，GitHub会显示设置说明。按照以下步骤操作：

### 方法一：使用HTTPS（推荐，简单）

1. 复制GitHub显示的HTTPS URL，例如：
   ```
   https://github.com/你的用户名/qt-translator.git
   ```

2. 在项目目录下执行以下命令：
   ```bash
   # 添加远程仓库
   git remote add origin https://github.com/你的用户名/qt-translator.git
   
   # 重命名分支为main（如果当前分支不是main）
   git branch -M main
   
   # 推送到GitHub
   git push -u origin main
   ```

### 方法二：使用SSH（需要配置SSH密钥）

如果你已经配置了SSH密钥，可以使用SSH URL：
```bash
git remote add origin git@github.com:你的用户名/qt-translator.git
git branch -M main
git push -u origin main
```

## 步骤三：验证上传结果

1. 刷新GitHub仓库页面
2. 应该能看到所有文件都已上传
3. 点击 **"Commits"** 标签，查看提交历史
4. 应该能看到所有的提交记录（当前有7次提交）

## 步骤四：截图提交记录

1. 在GitHub仓库页面，点击 **"Commits"** 标签
2. 截图整个提交历史页面
3. 也可以点击某个提交查看详细信息，截图提交详情

## 常见问题

### 问题1：提示需要身份验证

如果推送时提示需要输入用户名和密码：
- **GitHub已不再支持密码验证**，需要使用Personal Access Token
- 生成Token：
  1. GitHub → Settings → Developer settings → Personal access tokens → Tokens (classic)
  2. Generate new token (classic)
  3. 勾选 `repo` 权限
  4. 生成并复制Token
  5. 推送时，用户名输入你的GitHub用户名，密码输入Token

### 问题2：提示 "remote origin already exists"

如果已经添加过远程仓库，可以先删除再添加：
```bash
git remote remove origin
git remote add origin https://github.com/你的用户名/qt-translator.git
```

### 问题3：推送被拒绝

如果提示推送被拒绝，可能是因为GitHub仓库有README等文件。可以强制推送（谨慎使用）：
```bash
git push -u origin main --force
```

**建议**：先使用 `git pull origin main --allow-unrelated-histories` 合并后再推送

## 查看提交记录的命令

在本地也可以查看提交记录：

```bash
# 简洁格式
git log --oneline

# 图形化显示
git log --oneline --graph

# 详细信息
git log --pretty=format:"%h - %an, %ar : %s"

# 带文件变更统计
git log --stat

# 查看最近10条提交
git log -10 --oneline
```

## 完成后的截图要求

根据报告要求，需要提供：

1. **Git提交记录截图**
   - 在GitHub的Commits页面截图
   - 或者在本地使用 `git log --oneline --graph` 截图

2. **提交次数统计**
   - 当前提交次数：7次
   - 如需达到20次，可以按照"代码版本管理提交日志.md"中的说明拆分提交

3. **关键提交说明**
   - 已在"代码版本管理提交日志.md"中详细说明

