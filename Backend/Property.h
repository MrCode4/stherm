#pragma once

#include <QObject>

/*! ***********************************************************************************************
 * Here are just a few boilerplate definition of properties that helps to
 * declare a property (field, siganl, setter, getter, property defition etc)
 * in just a signle-line to keep the code clean and fresh in used classes.
 * PROPERTY_PRI to delcare a property with private setter,
 * PROPERTY_PRO to declare a property with protected setter,
 * PROPERTY_PUB to declare a property with public setter.
 * ************************************************************************************************/

#define PROPERTY(_SETTER_ACCCESS_, _TYPE_, _NAME_)\
Q_PROPERTY(_TYPE_ _NAME_ MEMBER m_##_NAME_ READ _NAME_ WRITE _NAME_ NOTIFY _NAME_##Changed)\
    Q_SIGNALS: void _NAME_##Changed(_TYPE_ value);\
    public: _TYPE_ _NAME_ () const {return m_##_NAME_;}\
    _SETTER_ACCCESS_:\
    void _NAME_(_TYPE_ value){\
        m_##_NAME_ = value;\
        emit _NAME_##Changed(value);\
}\
    private:\
    _TYPE_ m_##_NAME_;

#define PROPERTY_DEFAULT_VALUE(_SETTER_ACCCESS_, _TYPE_, _NAME_, _VALUE_)\
    Q_PROPERTY(_TYPE_ _NAME_ MEMBER m_##_NAME_ READ _NAME_ WRITE _NAME_ NOTIFY _NAME_##Changed)\
    Q_SIGNALS: void _NAME_##Changed(_TYPE_ value);\
    public: _TYPE_ _NAME_ () const {return m_##_NAME_;}\
    _SETTER_ACCCESS_:\
    void _NAME_(_TYPE_ value){\
        m_##_NAME_ = value;\
        emit _NAME_##Changed(value);\
    }\
    private:\
    _TYPE_ m_##_NAME_ = _VALUE_;

#define PROPERTY_PRI(_TYPE_, _NAME_) PROPERTY(private, _TYPE_, _NAME_)
#define PROPERTY_PRO(_TYPE_, _NAME_) PROPERTY(protected, _TYPE_, _NAME_)
#define PROPERTY_PUB(_TYPE_, _NAME_) PROPERTY(public, _TYPE_, _NAME_)

#define PROPERTY_PRI_DEF_VAL(_TYPE_, _NAME_, _VALUE_) PROPERTY_DEFAULT_VALUE(private, _TYPE_, _NAME_, _VALUE_)
#define PROPERTY_PRO_DEF_VAL(_TYPE_, _NAME_, _VALUE_) PROPERTY_DEFAULT_VALUE(protected, _TYPE_, _NAME_, _VALUE_)
#define PROPERTY_PUB_DEF_VAL(_TYPE_, _NAME_, _VALUE_) PROPERTY_DEFAULT_VALUE(public, _TYPE_, _NAME_, _VALUE_)

#define PROPERTY_LIST(_NAME_)\
    Q_PROPERTY(QVariantList _NAME_ MEMBER m_##_NAME_ NOTIFY _NAME_##Changed)\
    Q_PROPERTY(int _NAME_##Count READ _NAME_##Count NOTIFY _NAME_##CountChanged)\
    Q_SIGNALS: void _NAME_##Changed(QVariantList value);\
    Q_SIGNALS: void _NAME_##CountChanged(int value);\
    public:\
    int _NAME_##Count(){\
        return m_##_NAME_.count();\
    }\
    Q_INVOKABLE QVariant _NAME_##At(int index){\
        return index >= 0 && index < m_##_NAME_.count() ? m_##_NAME_[index] : QVariant();\
    }\
    void set##_NAME_(QVariantList value){\
        m_##_NAME_ = value;\
        emit _NAME_##Changed(value);\
    }\
    void append##_NAME_(QVariant value){\
        if (!m_##_NAME_.contains(value)){\
            m_##_NAME_.append(value);\
            emit _NAME_##CountChanged(m_##_NAME_.count());\
        }\
    }\
    void remove##_NAME_(QVariant value){\
        if (m_##_NAME_.contains(value)){\
            m_##_NAME_.removeAll(value);\
            emit _NAME_##CountChanged(m_##_NAME_.count());\
        }\
    }\
    void clear##_NAME_(){\
        if (!m_##_NAME_.empty()){\
            m_##_NAME_.clear();\
            emit _NAME_##CountChanged(m_##_NAME_.count());\
        }\
    }\
    protected:\
    QVariantList m_##_NAME_;
