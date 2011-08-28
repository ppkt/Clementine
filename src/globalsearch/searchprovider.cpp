/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "searchprovider.h"
#include "core/boundfuturewatcher.h"

#include <QtConcurrentRun>

const int SearchProvider::kArtHeight = 32;


SearchProvider::SearchProvider(const QString& name, const QIcon& icon,
                               QObject* parent)
  : QObject(parent),
    name_(name),
    icon_(icon)
{
}

QStringList SearchProvider::TokenizeQuery(const QString& query) {
  QStringList tokens(query.split(QRegExp("\\s+")));

  for (QStringList::iterator it = tokens.begin() ; it != tokens.end() ; ++it) {
    (*it).remove('(');
    (*it).remove(')');
    (*it).remove('"');

    const int colon = (*it).indexOf(":");
    if (colon != -1) {
      (*it).remove(0, colon + 1);
    }
  }

  return tokens;
}

int SearchProvider::TokenMatches(const QStringList& tokens, const QString& string) {
  int ret = 0;
  foreach (const QString& token, tokens) {
    if (string.contains(token, Qt::CaseInsensitive)) {
      ret ++;
    }
  }
  return ret;
}

BlockingSearchProvider::BlockingSearchProvider(const QString& name, const QIcon& icon, QObject* parent)
  : SearchProvider(name, icon, parent) {
}

void BlockingSearchProvider::SearchAsync(int id, const QString& query) {
  QFuture<ResultList> future = QtConcurrent::run(
      this, &BlockingSearchProvider::Search, id, query);

  BoundFutureWatcher<ResultList, int>* watcher =
      new BoundFutureWatcher<ResultList, int>(id);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(BlockingSearchFinished()));
}

void BlockingSearchProvider::BlockingSearchFinished() {
  BoundFutureWatcher<ResultList, int>* watcher =
      static_cast<BoundFutureWatcher<ResultList, int>*>(sender());
  watcher->deleteLater();

  const int id = watcher->data();
  emit ResultsAvailable(id, watcher->result());
  emit SearchFinished(id);
}
