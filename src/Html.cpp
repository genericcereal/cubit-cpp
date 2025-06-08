#include "Html.h"

Html::Html(int id, QObject *parent)
    : Element(HtmlType, id, parent)
{
    setName(QString("Html %1").arg(id));
}

void Html::setHtml(const QString &html)
{
    if (m_html != html) {
        m_html = html;
        emit htmlChanged();
        emit elementChanged();
    }
}

void Html::setUrl(const QString &url)
{
    if (m_url != url) {
        m_url = url;
        emit urlChanged();
        emit elementChanged();
    }
}