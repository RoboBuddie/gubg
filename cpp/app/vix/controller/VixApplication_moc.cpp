/****************************************************************************
** Meta object code from reading C++ file 'VixApplication.hpp'
**
** Created: Wed May 18 20:26:39 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "/home/gfannes/gubg/cpp/app/vix/controller/VixApplication.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VixApplication.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_vix__VixApplication[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x0a,
      48,   20,   20,   20, 0x0a,
      75,   73,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_vix__VixApplication[] = {
    "vix::VixApplication\0\0process4Commandline(QChar)\0"
    "process4Commandline(int)\0,\0"
    "setSelected(QModelIndex,QModelIndex)\0"
};

const QMetaObject vix::VixApplication::staticMetaObject = {
    { &QApplication::staticMetaObject, qt_meta_stringdata_vix__VixApplication,
      qt_meta_data_vix__VixApplication, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &vix::VixApplication::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *vix::VixApplication::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *vix::VixApplication::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_vix__VixApplication))
        return static_cast<void*>(const_cast< VixApplication*>(this));
    return QApplication::qt_metacast(_clname);
}

int vix::VixApplication::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QApplication::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: process4Commandline((*reinterpret_cast< QChar(*)>(_a[1]))); break;
        case 1: process4Commandline((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: setSelected((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
