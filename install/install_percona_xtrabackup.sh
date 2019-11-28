#!/bin/bash
#install_percona_xtrabackup.sh
#author:daviezhao
#date:2016-03-02

source ./comm.sh

echo "Installing percona-xtrabackup-24-2.4.1  ..."
installXtrabackup

echo "Installing percona-tookit-2.2.16 ..."
installPerconaToolkit
