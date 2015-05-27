/*
 * Copyright 2015 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Zsombor Egri <zsombor.egri@canonical.com>
 */

#include "ucstylehints.h"
#include "ucstyleditembase_p.h"
#include <QtQml/QQmlInfo>

void UCStyleHintsParser::verifyBindings(const QV4::CompiledData::Unit *qmlUnit, const QList<const QV4::CompiledData::Binding *> &bindings)
{
    Q_FOREACH(const QV4::CompiledData::Binding *binding, bindings) {
        verifyProperty(qmlUnit, binding);
    }
}

void UCStyleHintsParser::verifyProperty(const QV4::CompiledData::Unit *qmlUnit, const QV4::CompiledData::Binding *binding)
{
    if (binding->type == QV4::CompiledData::Binding::Type_Object) {
        error(qmlUnit->objectAt(binding->value.objectIndex), "StyleHints does not support creating state-specific objects.");
        return;
    }

    // group properties or attached properties, we do handle those as well
    if (binding->type == QV4::CompiledData::Binding::Type_GroupProperty
            || binding->type == QV4::CompiledData::Binding::Type_AttachedProperty) {
        const QV4::CompiledData::Object *subObj = qmlUnit->objectAt(binding->value.objectIndex);
        const QV4::CompiledData::Binding *subBinding = subObj->bindingTable();
        for (quint32 i = 0; i < subObj->nBindings; ++i, ++subBinding) {
            verifyProperty(qmlUnit, subBinding);
        }
    }

    // filter out signals!
    QString propertyName = qmlUnit->stringAt(binding->propertyNameIndex);
    if (propertyName.startsWith("on") && propertyName.at(2).isUpper()) {
        error(binding, "Signal properties are not supported.");
        return;
    }
}

void UCStyleHintsParser::applyBindings(QObject *obj, QQmlCompiledData *cdata, const QList<const QV4::CompiledData::Binding *> &bindings)
{
    UCStyleHints *hints = static_cast<UCStyleHints*>(obj);
    QV4::CompiledData::Unit *qmlUnit = cdata->compilationUnit->data;

    if (!hints->m_styledItem) {
        error(qmlUnit->objectAt(bindings[0]->value.objectIndex), "StyleHints not assigned to StyledItem.styleHints property.");
        return;
    }

    Q_FOREACH(const QV4::CompiledData::Binding *binding, bindings) {
        hints->decodeBinding(UCStyledItemBasePrivate::get(hints->m_styledItem)->styleItem, QString(), qmlUnit, binding);
    }

    hints->m_cdata = cdata;
    hints->m_decoded = true;
}

void UCStyleHints::decodeBinding(QQuickItem *styleInstance, const QString &propertyPrefix, const QV4::CompiledData::Unit *qmlUnit, const QV4::CompiledData::Binding *binding)
{
    QString propertyName = propertyPrefix + qmlUnit->stringAt(binding->propertyNameIndex);

    // handle grouped properties first
    if (binding->type == QV4::CompiledData::Binding::Type_GroupProperty
        || binding->type == QV4::CompiledData::Binding::Type_AttachedProperty) {

        const QV4::CompiledData::Object *subObj = qmlUnit->objectAt(binding->value.objectIndex);
        const QV4::CompiledData::Binding *subBinding = subObj->bindingTable();
        QString pre = propertyName + ".";
        for (quint32 i = 0; i < subObj->nBindings; ++i, ++subBinding) {
            decodeBinding(styleInstance, pre, qmlUnit, subBinding);
        }
        return;
    }

    if (styleInstance->metaObject()->indexOfProperty(qmlUnit->stringAt(binding->propertyNameIndex).toUtf8()) < 0) {
        QString warning("Style '%1' has no property called '%2'.");
        // TODO: show warning based on an env var
        qmlInfo(this) << warning.arg(UCStyledItemBasePrivate::get(m_styledItem)->styleName()).arg(propertyName);
        return;
    }

    switch (binding->type) {
    case QV4::CompiledData::Binding::Type_Script:
    {
        QString expression = binding->valueAsScriptString(qmlUnit);
        QUrl url = QUrl();
        int line = -1;
        int column = -1;

        QQmlData *ddata = QQmlData::get(this);
        if (ddata && ddata->outerContext && !ddata->outerContext->url.isEmpty()) {
            url = ddata->outerContext->url;
            line = ddata->lineNumber;
            column = ddata->columnNumber;
        }
        m_expressions << Expression(propertyName, binding->value.compiledScriptIndex, expression, url, line, column);
        break;
    }
    case QV4::CompiledData::Binding::Type_Translation:
    case QV4::CompiledData::Binding::Type_TranslationById:
    case QV4::CompiledData::Binding::Type_String:
    {
        m_values << qMakePair(propertyName, binding->valueAsString(qmlUnit));
        break;
    }
    case QV4::CompiledData::Binding::Type_Number:
    {
        m_values << qMakePair(propertyName, binding->valueAsNumber());
        break;
    }
    case QV4::CompiledData::Binding::Type_Boolean:
    {
        m_values << qMakePair(propertyName, binding->valueAsBoolean());
        break;
    }
    }
}

/*!
 *
 */
UCStyleHints::UCStyleHints(QObject *parent)
    : QObject(parent)
    , m_decoded(false)
    , m_completed(false)
{
}

void UCStyleHints::classBegin()
{
}

void UCStyleHints::componentComplete()
{
    if (!m_styledItem) {
        qmlInfo(this) << "StyleHints must be declared as property value for StyledItem or a derivate of it.";
    }
    if (parent()->findChildren<UCStyleHints*>().size() > 1) {
        qmlInfo(this) << "StyleHints cannot be declared as a standalone type, must be set as value for styleHints property.";
        return;
    }
    m_completed = true;
    _q_applyStyleHints();
}

void UCStyleHints::setStyledItem(UCStyledItemBase *item)
{
    m_styledItem = item;
    if (m_styledItem) {
        connect(m_styledItem, SIGNAL(themeChanged()),
                this, SLOT(_q_applyStyleHints()));
        connect(m_styledItem, SIGNAL(styleNameChanged()),
                this, SLOT(_q_applyStyleHints()));
        setParent(item);
    }
}

void UCStyleHints::unsetStyledItem()
{
    if (m_styledItem) {
        disconnect(m_styledItem, SIGNAL(themeChanged()),
                   this, SLOT(_q_applyStyleHints()));
        disconnect(m_styledItem, SIGNAL(styleNameChanged()),
                   this, SLOT(_q_applyStyleHints()));
    }
    m_styledItem.clear();
}

void UCStyleHints::_q_applyStyleHints()
{
    if (!m_completed || !m_decoded || !m_styledItem || !UCStyledItemBasePrivate::get(m_styledItem)->styleItem) {
        return;
    }

    QQuickItem *item = UCStyledItemBasePrivate::get(m_styledItem)->styleItem;
    QQmlContext *context = qmlContext(item);
    // apply values first
    for (int i = 0; i < m_values.size(); i++) {
        QQmlProperty::write(item, m_values[i].first, m_values[i].second, context);
    }

    // override context to use this context
    context = qmlContext(this);
    // then apply expressions/bindings
    for (int ii = 0; ii < m_expressions.count(); ii++) {
        Expression e = m_expressions[ii];
        QQmlProperty prop(item, e.name, qmlContext(item));
        if (!prop.isValid()) {
            continue;
        }

        // create a binding object from the expression using the palette context
        QQmlContextData *cdata = QQmlContextData::get(context);
        QQmlBinding *newBinding = 0;
        if (e.id != QQmlBinding::Invalid) {
            QV4::Scope scope(QQmlEnginePrivate::getV4Engine(qmlEngine(this)));
            QV4::ScopedValue function(scope, QV4::QmlBindingWrapper::createQmlCallableForFunction(cdata, item, m_cdata->compilationUnit->runtimeFunctions[e.id]));
            newBinding = new QQmlBinding(function, item, cdata);
        }
        if (!newBinding) {
            newBinding = new QQmlBinding(e.expression, item, cdata, e.url.toString(), e.line, e.column);
        }

        newBinding->setTarget(prop);
        QQmlAbstractBinding *prevBinding = QQmlPropertyPrivate::setBinding(prop, newBinding);
        if (prevBinding && prevBinding != newBinding) {
            prevBinding->destroy();
        }
    }
}

