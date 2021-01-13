/****************************************************************************
**
** Copyright (C) 2021 BogDan Vatra <bogdan@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qandroidbinder.h"

#include "qandroidbinder_p.h"
#include "qandroidparcel.h"
#include "qandroidparcel_p.h"

#include <jni/qandroidjnienvironment.h>

QT_BEGIN_NAMESPACE
QAndroidBinderPrivate::QAndroidBinderPrivate(QAndroidBinder *binder)
    : handle("org/qtproject/qt5/android/extras/QtAndroidBinder", "(J)V", jlong(binder))
    , m_isQtAndroidBinder(true)
{
    QAndroidJniExceptionCleaner cleaner;
}

QAndroidBinderPrivate::QAndroidBinderPrivate(const QAndroidJniObject &binder)
    : handle(binder)
    , m_isQtAndroidBinder(false)
{
}

QAndroidBinderPrivate::~QAndroidBinderPrivate()
{
    if (m_isQtAndroidBinder) {
        QAndroidJniExceptionCleaner cleaner;
        handle.callMethod<void>("setId", "(J)V", jlong(0));
        if (m_deleteListener)
            m_deleteListener();
    }
}

/*!
    \class QAndroidBinder
    \inmodule QtAndroidExtras
    \brief Wraps the most important methods of Android Binder class.

    The QAndroidBinder is a convenience class that wraps the most important
    \l {https://developer.android.com/reference/android/os/Binder.html}{Android Binder}
    methods.

    \since 5.10
*/


/*!
    \enum QAndroidBinder::CallType

    This enum is used with \l QAndroidBinder::transact() to describe the mode in which the
    IPC call is performed.

    \value Normal normal IPC, meaning that the caller waits the result from the callee
    \value OneWay one-way IPC, meaning that the caller returns immediately, without waiting for a result from the callee
*/

/*!
    Creates a new object which can be used to perform IPC.

    \sa onTransact, transact
 */
QAndroidBinder::QAndroidBinder()
    : d(new QAndroidBinderPrivate(this))
{
}

/*!
    Creates a new object from the \a binder Java object.

    \sa transact
 */
QAndroidBinder::QAndroidBinder(const QAndroidJniObject &binder)
    : d(new QAndroidBinderPrivate(binder))
{
}

QAndroidBinder::~QAndroidBinder()
{
}

/*!
    Default implementation is a stub that returns false.
    The user should override this method to get the transact data from the caller.

    The \a code is the action to perform.
    The \a data is the marshaled data sent by the caller.\br
    The \a reply is the marshaled data to be sent to the caller.\br
    The \a flags are the additional operation flags.\br

    \warning This method is called from Binder's thread which is different
    from the thread that this object was created.

    \sa transact
 */
bool QAndroidBinder::onTransact(int /*code*/, const QAndroidParcel &/*data*/, const QAndroidParcel &/*reply*/, CallType /*flags*/)
{
    return false;
}

/*!
    Performs an IPC call

    The \a code is the action to perform. Should be between
                \l {https://developer.android.com/reference/android/os/IBinder.html#FIRST_CALL_TRANSACTION}
                {FIRST_CALL_TRANSACTION} and
                \l {https://developer.android.com/reference/android/os/IBinder.html#LAST_CALL_TRANSACTION}
                {LAST_CALL_TRANSACTION}.\br
    The \a data is the marshaled data to send to the target.\br
    The \a reply (if specified) is the marshaled data to be received from the target.
    May be \b nullptr if you are not interested in the return value.\br
    The \a flags are the additional operation flags.\br

    \return true on success
 */
bool QAndroidBinder::transact(int code, const QAndroidParcel &data, QAndroidParcel *reply, CallType flags) const
{
    QAndroidJniExceptionCleaner cleaner;
    return d->handle.callMethod<jboolean>("transact", "(ILandroid/os/Parcel;Landroid/os/Parcel;I)Z",
                                   jint(code), data.d->handle.object(), reply ? reply->d->handle.object() : nullptr, jint(flags));
}

/*!
    The return value is useful to call other Java API which are not covered by this wrapper
 */
QAndroidJniObject QAndroidBinder::handle() const
{
    return d->handle;
}

QT_END_NAMESPACE
