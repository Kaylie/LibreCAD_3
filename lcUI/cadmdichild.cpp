#include "cmath"

#include "cadmdichild.h"

#include "cad/storage/documentimpl.h"

#include <QMenu>
#include <cad/operations/layerops.h>
#include <file.h>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>


using namespace lc::ui;
using namespace lc::viewer;

CadMdiChild::CadMdiChild(QWidget* parent) :
    QWidget(parent),
    _id(0),
    _activeLayer(nullptr) {

    if (this->objectName().isEmpty()) {
        this->setObjectName(QStringLiteral("CadMdiChild"));
    }

    this->resize(1078, 736);

    auto gridLayout = new QGridLayout(this);
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);

    _modelViewerImpl = new LCADModelViewerImpl(this);
    _viewer = _modelViewerImpl->getViewer();
    _viewer->setObjectName(QStringLiteral("viewer"));
    _viewer->setGeometry(QRect(50, 30, 581, 401));
    _viewer->setAutoFillBackground(true);
    _viewer->setContextMenuPolicy(Qt::CustomContextMenu);
    _viewer->setFocusPolicy(Qt::StrongFocus);
    connect(_viewer, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ctxMenu(const QPoint&)));

    gridLayout->addWidget(_viewer, 0, 0, 1, 1);

    _metaInfoManager = std::make_shared<lc::ui::MetaInfoManager>();
}

CadMdiChild::~CadMdiChild() {
	if(_destroyCallback) {
		_destroyCallback(_id);
	}
}


void CadMdiChild::newDocument() {
    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = std::make_shared<lc::storage::DocumentImpl>(storageManager());

    // Add the document to a LibreCAD Viewer system so we can visualize the document
    _modelViewerImpl->setDocument(_document);

    _activeLayer = _document->layerByName("0");
}


bool CadMdiChild::openFile() {
    auto availableTypes = lc::persistence::File::getSupportedFileExtensions();

    if(availableTypes.empty()) {
        QMessageBox::critical(nullptr, "Save error", "No library available for file opening.");
        return false;
    }

    //Build format string
    QString filterList="All Supported Formats(";
    auto it = availableTypes.begin();
    filterList += (" *."+it->first).c_str();
    it++;
    while(it != availableTypes.end()) {
        filterList += (" *."+it->first).c_str();
        it++;
    }
    filterList+=");;";

    it = availableTypes.begin();
    filterList += (it->second+"(*."+it->first+")").c_str();
    it++;
    while(it != availableTypes.end()) {
	    filterList += (";;"+it->second+"(*."+it->first+")").c_str();
        it++;
    }
    filterList+=";;All Files(*.*)";

    auto file = QFileDialog::getOpenFileName(nullptr,"Open document",nullptr,filterList);

    if(file == "") {
        return false;
    }

    auto fileInfo = QFileInfo(file);

    auto ext = fileInfo.suffix().toStdString();
    auto availableLibraries = lc::persistence::File::getAvailableLibrariesForFormat(ext);

    if(!availableLibraries.empty()) {
        //TODO: if more than once, ask which one to choose
        newDocument();
        _filename = file.toStdString();
        _fileType = lc::persistence::File::open(_document, _filename, availableLibraries.begin()->first);
    }
    else {
        QMessageBox::critical(nullptr, "Open error", "Unknown file extension ." + fileInfo.suffix());
        return false;
    }

    return true;
}

void CadMdiChild::saveFile(){
	if (_filename == "")saveAsFile();
	else lc::persistence::File::save(_document, _filename, _fileType);// @TODO Needs to fix it later
}

void CadMdiChild::saveAsFile() {
    QString filterList;
    QString selectedFilter;
    lc::persistence::File::Type type;
    auto availableTypes = lc::persistence::File::getAvailableFileTypes();

    if(availableTypes.empty()) {
        QMessageBox::critical(nullptr, "Save error", "No library available for file saving.");
        return;
    }

    auto it = availableTypes.begin();
    filterList = (it->second+"(*."+lc::persistence::File::getExtensionForFileType(it->first)+")").c_str();
    it++;
    while(it != availableTypes.end()) {
	    filterList += (";;"+it->second+"(*."+lc::persistence::File::getExtensionForFileType(it->first)+")").c_str();
        it++;
    }

    auto file = QFileDialog::getSaveFileName(nullptr, "Save file", "", filterList, &selectedFilter);

    auto selectedType = selectedFilter.toStdString();
    //Removing extension part
    std::size_t fpos = selectedType.rfind("(*");

    selectedType = selectedType.substr(0,fpos);

    for(auto availableType : availableTypes) {
        if(selectedType == availableType.second) {
            type = availableType.first;
            break;
        }
    }

    _fileType = type;

    //Add extension if not present
    auto fileInfo = QFileInfo(file);
    auto ext = fileInfo.suffix().toStdString();
    if(ext=="")file+=("."+lc::persistence::File::getExtensionForFileType(type)).c_str();
    _filename = file.toStdString();
    lc::persistence::File::save(_document, _filename, type);
}

void CadMdiChild::ctxMenu(const QPoint& pos) {
    auto menu = new QMenu;
    menu->addAction(tr("Test Item"), this, SLOT(test_slot()));
    menu->exec(_viewer->mapToGlobal(pos));
}

manager::SnapManager_SPtr  CadMdiChild::snapManager() const {
    return _modelViewerImpl->snapManager();
}

std::shared_ptr<lc::storage::Document> CadMdiChild::document() const {
    return _modelViewerImpl->document();
}

lc::storage::StorageManager_SPtr CadMdiChild::storageManager() const {
    return _modelViewerImpl->storageManager();
}

lc::storage::UndoManager_SPtr CadMdiChild::undoManager() const {
    return _modelViewerImpl->undoManager();
}

std::shared_ptr<drawable::Cursor> CadMdiChild::cursor() const {
	return _modelViewerImpl->cursor();
}

unsigned int CadMdiChild::id() {
	return _id;
}
void CadMdiChild::setId(unsigned int id) {
	_id = id;
}

void CadMdiChild::setDestroyCallback(LuaIntf::LuaRef destroyCallback) {
	_destroyCallback = std::move(destroyCallback);
}

void CadMdiChild::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);
    emit keyPressed(event);
}

drawable::TempEntities_SPtr CadMdiChild::tempEntities() {
    return _modelViewerImpl->tempEntities();
}

std::vector<lc::entity::CADEntity_CSPtr> CadMdiChild::selection() {
    return viewer()->documentCanvas()->selectedEntities().asVector();
}

lc::meta::Layer_CSPtr CadMdiChild::activeLayer() const {
    return _activeLayer;
}

void CadMdiChild::setActiveLayer(const lc::meta::Layer_CSPtr& activeLayer) {
    _activeLayer = activeLayer;
}

lc::ui::MetaInfoManager_SPtr CadMdiChild::metaInfoManager() const {
    return _metaInfoManager;
}

const manager::SnapManagerImpl_SPtr CadMdiChild::getSnapManager() const {
    return _modelViewerImpl->snapManager();
}
