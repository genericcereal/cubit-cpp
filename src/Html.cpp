#include "Html.h"

Html::Html(const QString &id, QObject *parent)
    : CanvasElement(HtmlType, id, parent)
{
    setName(QString("Html %1").arg(id.right(4)));  // Use last 4 digits for display
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