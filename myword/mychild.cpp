#include "mychild.h"
#include <QtWidgets>
#include <QByteArray>
#include <QTableWidget>
#include <QTableWidgetItem>

MyChild::MyChild()
{
    setAttribute(Qt::WA_DeleteOnClose);	//设置在子窗口关闭时销毁这个类的对象
        isUntitled = true;			//初始isUntitled为true

}
void MyChild::newFile()
{
    //设置窗口编号，因为编号会一直被保存，所以需要使用静态变量
    static int sequenceNumber = 1;
    //新建的文档默认未命名
    isUntitled = true;
    //将当前文件命名为"文档+编号"的形式，编号先使用再加1
    curFile = tr("文档 %1").arg(sequenceNumber++);
    //设置窗口标题，使用[*]可以在文档被更改后在文件名称后显示"*"号
    setWindowTitle(curFile + "[*]" + tr(" - Myself Word"));
    //文档更改时发送contentsChanged()信号，执行documentWasModified()槽函数
    connect(document(),SIGNAL(contentsChanged()),this
                ,SLOT(documentWasModified()));
}

void MyChild::documentWasModified()
{
    //根据文档的isModified()函数的返回值，判断编辑器内容是否被更改
    setWindowModified(document()->isModified());
}

QString MyChild::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

QString MyChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MyChild::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MyChild::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    //文件已经被保存过
    isUntitled = false;
    //文档没有被更改过
    document()->setModified(false);
    //窗口不显示被更改标志
    setWindowModified(false);
    //设置窗口标题，userFriendlyCurrentFile()返回文件名
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}
bool MyChild::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}
bool MyChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"), curFile,tr("HTML 文档 (*.htm *.html);;所有文件 (*.*)"));
    //获取文件路径，如果为空，则返回false，否则保存文件
    if (fileName.isEmpty())
        return false;
    return saveFile(fileName);
}
bool MyChild::saveFile(QString fileName)
{
    if (!(fileName.endsWith(".htm", Qt::CaseInsensitive) || fileName. endsWith(".html", Qt::CaseInsensitive))) {
        fileName += ".html"; //默认保存为 HTML 文档
    }
    QTextDocumentWriter writer(fileName);
    bool success = writer.write(this->document());
    if (success)
        setCurrentFile(fileName);
    return success;
}
bool MyChild::maybeSave()
{
    if (!document()->isModified())
        return true;
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Myself Qt Word"),tr("文档'%1'已被修改,保存吗?").arg(userFriendlyCurrentFile()),QMessageBox::Save | QMessageBox:: Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return save();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}
void MyChild::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = this->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    this->mergeCurrentCharFormat(format);
}
void MyChild::setAlign(int align)
{
    if (align == 1)
        this->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (align == 2)
        this->setAlignment(Qt::AlignHCenter);
    else if (align == 3)
        this->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (align == 4)
        this->setAlignment(Qt::AlignJustify);
}
void MyChild::setStyle(int style)
{
    QTextCursor cursor = this->textCursor();
    if (style != 0) {
        // QTextListFormat主要用于描述文本标号、编号的格式
        QTextListFormat::Style stylename = QTextListFormat::ListDisc;
        switch (style) {
            default:
            case 1:
                stylename = QTextListFormat::ListDisc;		//实心圆标号
                break;
            case 2:
                stylename = QTextListFormat::ListCircle;		//空心圆标号
                break;
            case 3:
                stylename = QTextListFormat::ListSquare;		//方形标号
                break;
            case 4:
                stylename = QTextListFormat::ListDecimal; 	//十进制编号
                break;
            case 5:
                stylename = QTextListFormat::ListLowerAlpha;   //小写字母编号
                break;
            case 6:
                stylename = QTextListFormat::ListUpperAlpha; //大写字母编号
                break;
            case 7:
                stylename = QTextListFormat::ListLowerRoman; //小写罗马编号
                break;
            case 8:
                stylename = QTextListFormat::ListUpperRoman; //大写罗马编号
                break;
        }
        //为支持撤销操作
        cursor.beginEditBlock();
        QTextBlockFormat blockFmt = cursor.blockFormat();
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }
        listFmt.setStyle(stylename);
        cursor.createList(listFmt);
        cursor.endEditBlock();
    } else {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);						//合并格式
    }
}
bool MyChild::loadFile(const QString &fileName){
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly)){
        QMessageBox msg;
        msg.setText("error !");
        msg.show();
    }
    QByteArray t = file.readAll();
    QTextCodec *codec = Qt::codecForHtml (t);
    QString str = codec->toUnicode (t);
     this->setHtml (str);
    setCurrentFile(fileName);
    connect(document (), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));
    return true;
}
void MyChild::textTable(){
    QTableWidget* table = new QTableWidget(this); //创建一个表格
    table->setRowCount(3); //设置行数
    table->setColumnCount(2); //设置列数
    table->setHorizontalHeaderLabels(QStringList()<<"column1"<<"column2"); //设置行头
    table->setItem(0,0,new QTableWidgetItem("item1")); //设置表格内容
    table->show();
}
