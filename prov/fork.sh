#!/bin/bash

/vagrant/prov/server.sh 1 &
/vagrant/prov/server.sh 2 &

echo "Server starter worker running."
