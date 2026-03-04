#pragma once

#include "Aliases.hpp"

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class WhitespaceHighlighter final : public QSyntaxHighlighter {
    Q_OBJECT

   public:
    explicit WhitespaceHighlighter(bool enabled, QTextDocument* doc);

   protected:
    void highlightBlock(const QString& text) override;

   private:
    QTextCharFormat format;
    bool enabled;
};