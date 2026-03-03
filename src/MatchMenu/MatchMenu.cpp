#include "MatchMenu.hpp"

#include "ColoredTextLabel.hpp"
#include "rpgmtranslate.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>

struct SearchMatch {
    const u32 start;
    const u32 len;
    const f32 score;
};

MatchMenu::MatchMenu(QWidget* const parent) : QDockWidget(parent) {
    hide();
}

void MatchMenu::init(
    QPushButton* const clearButton,
    QTableWidget* const matchTable
) {
    this->clearButton = clearButton;
    this->matchTable = matchTable;

    connect(clearButton, &QPushButton::pressed, this, [this] -> void {
        this->matchTable->model()->removeRows(
            0,
            this->matchTable->model()->rowCount()
        );
    });
}

void MatchMenu::appendMatch(
    const QString& filename,
    const u32 lineIndex,
    const QString& termSource,
    const QString& termTranslation,
    const QStringView source,
    const QStringView translation,
    const ByteBuffer matches
) {
    if (matches.len == 0) {
        return;
    }

    const i32 row = matchTable->rowCount();
    matchTable->insertRow(row);

    auto* const filenameItem = new QTableWidgetItem(filename);
    matchTable->setItem(row, 0, filenameItem);

    auto* const rowItem = new QTableWidgetItem(QString::number(lineIndex + 1));
    matchTable->setItem(row, 1, rowItem);

    const u8* ptr = matches.ptr;

    u32 sourceCount = *ras<const u32*>(ptr);
    ptr += sizeof(u32);

    auto sourceMatches = span(ras<const SearchMatch*>(ptr), sourceCount);
    ptr += sourceCount * sizeof(SearchMatch);

    u32 translationCount = *ras<const u32*>(ptr);
    ptr += sizeof(u32);

    auto translationMatches =
        span(ras<const SearchMatch*>(ptr), translationCount);
    ptr += translationCount * sizeof(SearchMatch);

    QStringList sourceMatchDescs;
    vector<Span> sourceSpans;

    sourceMatchDescs.reserve(sourceCount);
    sourceSpans.reserve(sourceCount);

    for (const auto match : sourceMatches) {
        if (match.score == 0.0F) {
            sourceMatchDescs.append(tr("Exact"));
        } else {
            sourceMatchDescs.append(
                tr("Fuzzy (%1)").arg(QString::number(match.score, 10, 3))
            );
        }

        sourceSpans.emplace_back(match.start, match.len);
    }

    QStringList translationMatchDescs;
    vector<Span> translationSpans;

    translationMatchDescs.reserve(translationCount);
    translationSpans.reserve(translationCount);

    for (const auto match : translationMatches) {
        if (match.score == 0.0F) {
            translationMatchDescs.append(tr("Exact"));
        } else {
            translationMatchDescs.append(
                tr("Fuzzy (%1)").arg(QString::number(match.score, 10, 3))
            );
        }

        translationSpans.emplace_back(match.start, match.len);
    }

    auto* const termOccurrencesItem =
        new QTableWidgetItem(tr("%1, %2 occurrences, %3")
                                 .arg(termSource)
                                 .arg(sourceCount)
                                 .arg(sourceMatchDescs.join(" ,"_L1)));
    matchTable->setItem(row, 2, termOccurrencesItem);

    auto* const translationOccurrencesItem =
        new QTableWidgetItem(tr("%1, %2 occurrences, %3")
                                 .arg(termTranslation)
                                 .arg(translationCount)
                                 .arg(translationMatchDescs.join(" ,"_L1)));
    matchTable->setItem(row, 3, translationOccurrencesItem);

    QColor highlightedColor =
        qApp->palette().color(QPalette::Active, QPalette::Highlight);
    highlightedColor.setAlphaF(0.85F);

    // TODO: ColoredTextLabel doesn't show anything here
    auto* const sourceMatchWidget = new ColoredTextLabel(matchTable);
    sourceMatchWidget->setData(source, sourceSpans, highlightedColor);
    matchTable->setCellWidget(row, 4, sourceMatchWidget);

    auto* const translationMatchWidget = new ColoredTextLabel(matchTable);
    translationMatchWidget
        ->setData(translation, translationSpans, highlightedColor);
    matchTable->setCellWidget(row, 5, translationMatchWidget);

    QString info;
    const bool match = sourceCount <= translationCount;

    if (translation.isEmpty()) {
        info = tr("Translation is empty.");
    } else if (translationCount == 0) {
        info = tr("Term translation is not present.");
    } else if (!match) {
        info = tr(
            "Number of term occurrences doesn't match the number of translation occurrences."
        );
    } else {
        info = tr("Match.");
    }

    auto* const infoItem = new QTableWidgetItem(info);
    matchTable->setItem(row, 6, infoItem);

    matchTable->resizeColumnsToContents();
}

void MatchMenu::clear() {
    matchTable->model()->removeRows(0, matchTable->model()->rowCount());
}