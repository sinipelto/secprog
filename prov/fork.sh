#!/bin/bash

/vagrant/prov/server.sh 1 >> /vagrant/prov/serv1.log 2>&1 &
/vagrant/prov/server.sh 2 >> /vagrant/prov/serv2.log 2>&1 &

echo "Server starter worker running."
