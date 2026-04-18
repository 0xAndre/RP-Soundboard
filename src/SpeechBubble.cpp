// src/SpeechBubble.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "SpeechBubble.h"

#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QTextOption>
#include <QTimer>
#include <QToolButton>

namespace
{
constexpr int kShadowMargin = 12;
constexpr int kCornerRadius = 10;
constexpr int kTipHeight = 10;
constexpr int kTipHalfWidth = 9;
constexpr int kTipOffsetFromLeft = 24; // distance from body's left edge to tip center
constexpr int kTextPadding = 12;
constexpr int kAttachGap = 4;
constexpr int kCloseButtonSize = 22;
constexpr int kCloseButtonMargin = 4;
} // namespace


SpeechBubble::SpeechBubble(QWidget* parent /*= 0*/) :
	BaseClass(parent, Qt::FramelessWindowHint),
	m_attach(nullptr),
	m_closeButton(nullptr),
	m_closable(true),
	m_bubbleStyle(true),
	m_backgroundColor(QColor(255, 183, 59))
{
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_ShowWithoutActivating);
	if (parent)
		parent->installEventFilter(this);

	auto* shadow = new QGraphicsDropShadowEffect(this);
	shadow->setBlurRadius(20);
	shadow->setOffset(0, 2);
	shadow->setColor(QColor(0, 0, 0, 120));
	setGraphicsEffect(shadow);

	m_closeButton = new QToolButton(this);
	m_closeButton->setFocusPolicy(Qt::NoFocus);
	m_closeButton->setCursor(Qt::PointingHandCursor);
	m_closeButton->setFixedSize(kCloseButtonSize, kCloseButtonSize);
	m_closeButton->setText(QStringLiteral("\u2715"));
	m_closeButton->setStyleSheet(
		"QToolButton { border: none; border-radius: 11px; background: transparent;"
		" color: #2b2b2b; font-weight: bold; font-size: 12px; }"
		"QToolButton:hover { background: rgba(0, 0, 0, 40); }"
		"QToolButton:pressed { background: rgba(0, 0, 0, 70); }"
	);
	connect(m_closeButton, &QToolButton::clicked, this, [this]() {
		emit closePressed();
		hide();
		deleteLater();
	});
}


void SpeechBubble::setText(const QString& text)
{
	m_text = text;
	update();
}


void SpeechBubble::paintEvent(QPaintEvent* /*evt*/)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	const qreal bodyLeft = kShadowMargin;
	const qreal bodyTop = kShadowMargin + (m_bubbleStyle ? kTipHeight : 0);
	const qreal bodyRight = width() - kShadowMargin;
	const qreal bodyBottom = height() - kShadowMargin;
	const qreal r = kCornerRadius;

	QPainterPath path;
	if (m_bubbleStyle)
	{
		const qreal tipCenterX = bodyLeft + kTipOffsetFromLeft;
		const qreal tipLeftX = tipCenterX - kTipHalfWidth;
		const qreal tipRightX = tipCenterX + kTipHalfWidth;
		const qreal tipTopY = bodyTop - kTipHeight;

		path.moveTo(bodyLeft + r, bodyTop);
		path.lineTo(tipLeftX, bodyTop);
		path.lineTo(tipCenterX, tipTopY);
		path.lineTo(tipRightX, bodyTop);
		path.lineTo(bodyRight - r, bodyTop);
		path.quadTo(bodyRight, bodyTop, bodyRight, bodyTop + r);
		path.lineTo(bodyRight, bodyBottom - r);
		path.quadTo(bodyRight, bodyBottom, bodyRight - r, bodyBottom);
		path.lineTo(bodyLeft + r, bodyBottom);
		path.quadTo(bodyLeft, bodyBottom, bodyLeft, bodyBottom - r);
		path.lineTo(bodyLeft, bodyTop + r);
		path.quadTo(bodyLeft, bodyTop, bodyLeft + r, bodyTop);
		path.closeSubpath();
	}
	else
	{
		path.addRoundedRect(QRectF(bodyLeft, bodyTop, bodyRight - bodyLeft, bodyBottom - bodyTop), r, r);
	}

	painter.setPen(QPen(m_backgroundColor.darker(135), 1));
	painter.setBrush(m_backgroundColor);
	painter.drawPath(path);

	QTextOption opt;
	opt.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	opt.setWrapMode(QTextOption::WordWrap);
	const qreal textRightInset = m_closable ? (kCloseButtonSize + kCloseButtonMargin) : kTextPadding;
	QRectF textRect(
		bodyLeft + kTextPadding, bodyTop + kTextPadding / 2, bodyRight - bodyLeft - kTextPadding - textRightInset,
		bodyBottom - bodyTop - kTextPadding
	);
	painter.setPen(QColor(40, 40, 40));
	painter.drawText(textRect, m_text, opt);
}


void SpeechBubble::resizeEvent(QResizeEvent* evt)
{
	BaseClass::resizeEvent(evt);
	updateCloseButtonPos();
}


void SpeechBubble::attachTo(QWidget* widget)
{
	m_attach = widget;
	m_attach->installEventFilter(this);
	recalcPos();
	updateCloseButtonPos();
}


void SpeechBubble::setBackgroundColor(const QColor& color)
{
	m_backgroundColor = color;
	update();
}


void SpeechBubble::setClosable(bool closable)
{
	m_closable = closable;
	if (m_closeButton)
		m_closeButton->setVisible(closable);
	update();
}


void SpeechBubble::setBubbleStyle(bool bubbleStyle)
{
	m_bubbleStyle = bubbleStyle;
	updateCloseButtonPos();
	update();
}


bool SpeechBubble::eventFilter(QObject* object, QEvent* evt)
{
	if (object == parent())
	{
		switch (evt->type())
		{
		case QEvent::Move:
		case QEvent::Resize:
			recalcPos();
			break;
		case QEvent::Close:
			close();
			break;
		case QEvent::Hide:
			hide();
			break;
		case QEvent::Show:
			QTimer::singleShot(0, this, [this]() {
				this->recalcPos();
				this->show();
			});
			break;
		default:
			break;
		}
	}

	return BaseClass::eventFilter(object, evt);
}


void SpeechBubble::recalcPos()
{
	if (!m_attach)
		return;

	QPoint pos;
	if (m_bubbleStyle)
	{
		const int tipLocalCenterX = kShadowMargin + kTipOffsetFromLeft;
		const int tipLocalTopY = kShadowMargin;
		const QPoint attachBottomCenter = m_attach->mapToGlobal(QPoint(m_attach->width() / 2, m_attach->height()));
		pos = QPoint(attachBottomCenter.x() - tipLocalCenterX, attachBottomCenter.y() + kAttachGap - tipLocalTopY);
	}
	else
	{
		const QPoint attachCenter = m_attach->mapToGlobal(QPoint(m_attach->width() / 2, m_attach->height() / 2));
		pos = QPoint(attachCenter.x() - width() / 2, attachCenter.y() - height() / 2);
	}

	this->move(pos);
}


void SpeechBubble::updateCloseButtonPos()
{
	if (!m_closeButton)
		return;
	const int bodyTop = kShadowMargin + (m_bubbleStyle ? kTipHeight : 0);
	const int bodyRight = width() - kShadowMargin;
	m_closeButton->move(bodyRight - kCloseButtonSize - kCloseButtonMargin, bodyTop + kCloseButtonMargin);
}
