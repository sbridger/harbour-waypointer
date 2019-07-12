#include "waypoints.h"

#include <QStandardPaths>
#include <QDir>
#include <QSaveFile>
#include <QXmlStreamWriter>
#include <QDebug>

Waypoints::Waypoints(QObject *parent) : QObject(parent)
{
//  waypoints = new QVector<waypoint>();
}

QString Waypoints::addWaypoint(QString description, QGeoCoordinate pos) {
  waypoint waypoint;
  waypoint.name = description;
  waypoint.lat = pos.latitude();
  waypoint.lon = pos.longitude();
  waypoint.time = QDateTime::currentDateTime();
  waypoints.append(waypoint);
  return getLastlog();
}

QString Waypoints::removeLastWaypoint() {
  if (waypoints.isEmpty()) {
    return "";
  }
  waypoints.removeLast();
  return getLastlog();
}

QString Waypoints::getLastlog() {
  // FIXME: addWaypoint could just pass on the just appended waypoint as an argument.
  if (waypoints.isEmpty()) {
    return "–";
  }
  waypoint waypoint;
  waypoint = waypoints.last();

  return waypoint.name + "@" + waypoint.time.toString("hh.mm.ss");
}

bool Waypoints::saveWaypoints(QString name) {
    // Used the Rena save code (GPLv3)
    // https://github.com/Simoma/rena/blob/master/src/trackrecorder.cpp
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString subDir = "Waypointer";
    QString filename;
    filename = waypoints.at(0).time.toString(Qt::ISODate) + " - " + name + ".gpx";
    qDebug()<<"File:"<<homeDir<<"/"<<subDir<<"/"<<filename;

    QDir home = QDir(homeDir);
    if(!home.exists(subDir)) {
        qDebug()<<"Directory does not exist, creating";
        if(home.mkdir(subDir)) {
            qDebug()<<"Directory created";
        } else {
            qDebug()<<"Directory creation failed, aborting";
            return false;
        }
    }

    QSaveFile file;
    file.setFileName(homeDir + "/" + subDir + "/" + filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug()<<"File opening failed, aborting";
        return false;
    }

    QXmlStreamWriter xml;
    xml.setDevice(&file);
    xml.setAutoFormatting(true);    // Human readable output
    xml.writeStartDocument();
    xml.writeDefaultNamespace("http://www.topografix.com/GPX/1/1");
    xml.writeStartElement("gpx");
    xml.writeAttribute("version", "1.1");
    xml.writeAttribute("Creator", "Waypointer for Sailfish");

    xml.writeStartElement("metadata");
    xml.writeTextElement("name", name);
    xml.writeEndElement(); // metadata

    for(int i=0 ; i < waypoints.size(); i++) {
        xml.writeStartElement("wpt");
        xml.writeAttribute("lat", QString::number(waypoints.at(i).lat, 'g', 15));
        xml.writeAttribute("lon", QString::number(waypoints.at(i).lon, 'g', 15));
        xml.writeTextElement("time", waypoints.at(i).time.toUTC().toString(Qt::ISODate));
        xml.writeTextElement("name", waypoints.at(i).name);
        xml.writeEndElement();
    }

    xml.writeEndElement(); // gpx
    xml.writeEndDocument();

    file.commit();
    if(file.error()) {
        qDebug()<<"Error in writing to a file";
        qDebug()<<file.errorString();
    } else {
        QDir renaDir = QDir(homeDir + "/" + subDir);
        renaDir.remove("Autosave");
    }
  return true;
}
