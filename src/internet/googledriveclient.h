/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#ifndef GOOGLEDRIVECLIENT_H
#define GOOGLEDRIVECLIENT_H

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QUrl>
#include <QVariantMap>

class OAuthenticator;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;


namespace google_drive {

class Client;

// Holds the metadata for a file on Google Drive.
class File {
public:
  File(const QVariantMap& data = QVariantMap()) : data_(data) {}

  QString id() const { return data_["id"].toString(); }
  QString etag() const { return data_["etag"].toString(); }
  QString title() const { return data_["title"].toString(); }
  long size() const { return data_["fileSize"].toUInt(); }
  QUrl download_url() const { return data_["downloadUrl"].toUrl(); }

  QDateTime modified_date() const {
    return QDateTime::fromString(data_["modifiedDate"].toString(), Qt::ISODate);
  }

  QDateTime created_date() const {
    return QDateTime::fromString(data_["createdDate"].toString(), Qt::ISODate);
  }

private:
  QVariantMap data_;
};

typedef QList<File> FileList;


class ConnectResponse : public QObject {
  Q_OBJECT
  friend class Client;

public:
  const QString& refresh_token() const { return refresh_token_; }

signals:
  void Finished();

private:
  ConnectResponse(QObject* parent);
  QString refresh_token_;
};


class ListFilesResponse : public QObject {
  Q_OBJECT
  friend class Client;

public:
  const QString& query() const { return query_; }

signals:
  void FilesFound(const QList<google_drive::File>& files);
  void Finished();

private:
  ListFilesResponse(const QString& query, QObject* parent);
  QString query_;
};


class GetFileResponse : public QObject {
  Q_OBJECT
  friend class Client;

public:
  const QString& file_id() const { return file_id_; }
  const File& file() const { return file_; }

signals:
  void Finished();

private:
  GetFileResponse(const QString& file_id, QObject* parent);
  QString file_id_;
  File file_;
};


class Client : public QObject {
  Q_OBJECT

public:
  Client(QObject* parent = 0);

  bool is_authenticated() const { return !access_token_.isEmpty(); }
  const QString& access_token() const { return access_token_; }

  ConnectResponse* Connect(const QString& refresh_token = QString());
  ListFilesResponse* ListFiles(const QString& query);
  GetFileResponse* GetFile(const QString& file_id);

private slots:
  void ConnectFinished(ConnectResponse* response, OAuthenticator* oauth);
  void ListFilesFinished(ListFilesResponse* response, QNetworkReply* reply);
  void GetFileFinished(GetFileResponse* response, QNetworkReply* reply);

private:
  void AddAuthorizationHeader(QNetworkRequest* request) const;
  void MakeListFilesRequest(ListFilesResponse* response,
                            const QString& page_token = QString());
  
private:
  QNetworkAccessManager* network_;

  QString access_token_;
};

} // namespace

#endif // GOOGLEDRIVECLIENT_H