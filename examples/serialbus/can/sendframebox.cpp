// Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "sendframebox.h"
#include "ui_sendframebox.h"

constexpr char THREE_DIGITS[] = "[[:xdigit:]]{3}";

enum {
    MaxStandardId = 0x7FF,
    MaxExtendedId = 0x10000000
};

enum {
    MaxPayload = 8,
    MaxPayloadFd = 64
};

bool isEvenHex(QString input)
{
    const QChar space = QLatin1Char(' ');
    input.remove(space);

    if (input.size() % 2)
        return false;

    return true;
}

// Formats a string of hex characters with a space between every byte
// Example: "012345" -> "01 23 45"
static QString formatHexData(const QString &input)
{
    const QChar space = QLatin1Char(' ');
    QString out = input;

    const QRegularExpression threeDigits(THREE_DIGITS);
    const QRegularExpression oneDigitAndSpace(QStringLiteral("((\\s+)|^)([[:xdigit:]]{1})(\\s+)"));

    while (oneDigitAndSpace.match(out).hasMatch() || threeDigits.match(out).hasMatch()) {
        if (threeDigits.match(out).hasMatch()) {
            const QRegularExpressionMatch match = threeDigits.match(out);
            out.insert(match.capturedEnd() - 1, space);
        } else if (oneDigitAndSpace.match(out).hasMatch()) {
            const QRegularExpressionMatch match = oneDigitAndSpace.match(out);
            if (out.at(match.capturedEnd() - 1) == space)
                out.remove(match.capturedEnd() - 1, 1);
        }
    }

    return out.simplified().toUpper();
}

HexIntegerValidator::HexIntegerValidator(QObject *parent) :
    QValidator(parent),
    m_maximum(MaxStandardId)
{
}

QValidator::State HexIntegerValidator::validate(QString &input, int &) const
{
    bool ok;
    uint value = input.toUInt(&ok, 16);

    if (input.isEmpty())
        return Intermediate;

    if (!ok || value > m_maximum)
        return Invalid;

    return Acceptable;
}

void HexIntegerValidator::setMaximum(uint maximum)
{
    m_maximum = maximum;
}

HexStringValidator::HexStringValidator(QObject *parent) :
    QValidator(parent),
    m_maxLength(MaxPayload)
{
}

QValidator::State HexStringValidator::validate(QString &input, int &pos) const
{
    const int maxSize = 2 * m_maxLength;
    const QChar space = QLatin1Char(' ');
    QString data = input;

    data.remove(space);

    if (data.isEmpty())
        return Intermediate;

    // limit maximum size
    if (data.size() > maxSize)
        return Invalid;

    // check if all input is valid
    const QRegularExpression re(QStringLiteral("^[[:xdigit:]]*$"));
    if (!re.match(data).hasMatch())
        return Invalid;

    // don't allow user to enter more than one space
    const QRegularExpression twoSpaces(QStringLiteral("([\\s]{2})"));
    if (twoSpaces.match(input).hasMatch()) {
        const QRegularExpressionMatch match = twoSpaces.match(input);
        input.replace(match.capturedStart(), 2, ' ');
        pos = match.capturedEnd() - 1;
    }

    // insert a space after every two hex nibbles
    const QRegularExpression threeDigits(THREE_DIGITS);

    while (threeDigits.match(input).hasMatch()) {
        const QRegularExpressionMatch match = threeDigits.match(input);
        if (pos == match.capturedStart() + 1) {
            // add one hex nibble before two - Abc
            input.insert(match.capturedStart() + 1, space);
            pos = match.capturedStart() + 1;
        } else if (pos == match.capturedStart() + 2) {
            // add hex nibble in the middle - aBc
            input.insert(match.capturedEnd() - 1, space);
            pos = match.capturedEnd();
        } else {
            // add one hex nibble after two - abC
            input.insert(match.capturedEnd() - 1, space);
            pos = match.capturedEnd() + 1;
        }
    }

    return Acceptable;
}

void HexStringValidator::setMaxLength(int maxLength)
{
    m_maxLength = maxLength;
}

SendFrameBox::SendFrameBox(QWidget *parent) :
    QGroupBox(parent),
    m_ui(new Ui::SendFrameBox)
{
    m_ui->setupUi(this);

    m_hexIntegerValidator = new HexIntegerValidator(this);
    m_ui->frameIdEdit->setValidator(m_hexIntegerValidator);
    m_hexStringValidator = new HexStringValidator(this);
    m_ui->payloadEdit->setValidator(m_hexStringValidator);

    connect(m_ui->dataFrame, &QRadioButton::toggled, this, [this](bool set) {
        if (set)
            m_ui->flexibleDataRateBox->setEnabled(true);
    });

    connect(m_ui->remoteFrame, &QRadioButton::toggled, this, [this](bool set) {
        if (set) {
            m_ui->flexibleDataRateBox->setEnabled(false);
            m_ui->flexibleDataRateBox->setChecked(false);
        }
    });

    connect(m_ui->errorFrame, &QRadioButton::toggled, this, [this](bool set) {
        if (set) {
            m_ui->flexibleDataRateBox->setEnabled(false);
            m_ui->flexibleDataRateBox->setChecked(false);
        }
    });

    connect(m_ui->extendedFormatBox, &QCheckBox::toggled, this, [this](bool set) {
        m_hexIntegerValidator->setMaximum(set ? MaxExtendedId : MaxStandardId);
    });

    connect(m_ui->flexibleDataRateBox, &QCheckBox::toggled, this, [this](bool set) {
        m_hexStringValidator->setMaxLength(set ? MaxPayloadFd : MaxPayload);
        m_ui->bitrateSwitchBox->setEnabled(set);
        if (!set)
            m_ui->bitrateSwitchBox->setChecked(false);
    });

    auto frameIdOrPayloadChanged = [this]() {
        const bool hasFrameId = !m_ui->frameIdEdit->text().isEmpty();
        m_ui->sendButton->setEnabled(hasFrameId);
        m_ui->sendButton->setToolTip(hasFrameId
                                     ? QString() : tr("Cannot send because no Frame ID was given."));
        if (hasFrameId) {
            const bool isEven = isEvenHex(m_ui->payloadEdit->text());
            m_ui->sendButton->setEnabled(isEven);
            m_ui->sendButton->setToolTip(isEven
                                         ? QString() : tr("Cannot send because Payload hex string is invalid."));
        }
    };
    connect(m_ui->frameIdEdit, &QLineEdit::textChanged, frameIdOrPayloadChanged);
    connect(m_ui->payloadEdit, &QLineEdit::textChanged, frameIdOrPayloadChanged);
    frameIdOrPayloadChanged();

    connect(m_ui->sendButton, &QPushButton::clicked, [this]() {
        const uint frameId = m_ui->frameIdEdit->text().toUInt(nullptr, 16);
        QString data = m_ui->payloadEdit->text();
        m_ui->payloadEdit->setText(formatHexData(data));
        const QByteArray payload = QByteArray::fromHex(data.remove(QLatin1Char(' ')).toLatin1());

        QCanBusFrame frame = QCanBusFrame(frameId, payload);
        frame.setExtendedFrameFormat(m_ui->extendedFormatBox->isChecked());
        frame.setFlexibleDataRateFormat(m_ui->flexibleDataRateBox->isChecked());
        frame.setBitrateSwitch(m_ui->bitrateSwitchBox->isChecked());

        if (m_ui->errorFrame->isChecked())
            frame.setFrameType(QCanBusFrame::ErrorFrame);
        else if (m_ui->remoteFrame->isChecked())
            frame.setFrameType(QCanBusFrame::RemoteRequestFrame);

        emit sendFrame(frame);
    });
}

SendFrameBox::~SendFrameBox()
{
    delete m_ui;
}
