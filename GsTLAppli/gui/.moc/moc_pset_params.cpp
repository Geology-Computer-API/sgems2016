/****************************************************************************
** Meta object code from reading C++ file 'pset_params.h'
**
** Created: Thu 23. Jun 17:50:30 2016
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../variogram2/pset_params.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pset_params.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Pset_params[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      32,   12,   12,   12, 0x0a,
      50,   12,   12,   12, 0x0a,
      68,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Pset_params[] = {
    "Pset_params\0\0parameterChanged()\0"
    "load_parameters()\0save_parameters()\0"
    "set_directions_count(int)\0"
};

const QMetaObject Pset_params::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Pset_params,
      qt_meta_data_Pset_params, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Pset_params::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Pset_params::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Pset_params::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Pset_params))
        return static_cast<void*>(const_cast< Pset_params*>(this));
    if (!strcmp(_clname, "Ui::Point_set_base"))
        return static_cast< Ui::Point_set_base*>(const_cast< Pset_params*>(this));
    return QWidget::qt_metacast(_clname);
}

int Pset_params::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: parameterChanged(); break;
        case 1: load_parameters(); break;
        case 2: save_parameters(); break;
        case 3: set_directions_count((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Pset_params::parameterChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
