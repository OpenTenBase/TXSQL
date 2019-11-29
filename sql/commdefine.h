/*
 * commdefine.h
 *
 *  Created on: 2014/4/6
 *      Author: harlylei
 */

#ifndef COMMDEFILE_H_
#define COMMDEFILE_H_

//just for eclipse
#ifndef NotJustForEclipse

    #define ENABLED_DEBUG_SYNC

    #define HAVE_DLOPEN

    #define MYSQL_SERVER  //by harlylei
    #define HAVE_REPLICATION //by harlylei

    #define MUTEX_FUTEX
    #define HAVE_IB_LINUX_FUTEX

    #undef MYSQL_CLIENT

#endif

#endif /* COMMDEFILE_H_ */
