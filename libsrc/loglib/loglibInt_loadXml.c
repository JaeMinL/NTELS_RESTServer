#ifndef LOGLIB_XML_CONFIG_DISABLE

#include <stdio.h>
#include <syslog.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "loglib.h"
#include "loglib.x"
#include "loglibInt.h"
#include "loglibInt.x"

FT_PRIVATE RT_RESULT      cfg_loadApnd          (xmlNodePtr node, LoglibIntApndCfg **rt_apndCfg);
FT_PRIVATE RT_RESULT      cfg_loadLogCfg        (xmlNodePtr node, xmlChar *name);

FT_PRIVATE RT_RESULT cfg_loadApnd(xmlNodePtr node, LoglibIntApndCfg **rt_apndCfg)
{
    SINT ret = 0;
    BOOL stdTypeFlg = RC_FALSE;
    BOOL facFlg = RC_FALSE;
    BOOL filePathFlg = RC_FALSE;
    BOOL logNameFlg = RC_FALSE;
    BOOL logSizeFlg = RC_FALSE;
    UINT apndType = 0;
    xmlAttrPtr attr = NULL;
    xmlNodePtr apndNode = NULL;
    LoglibIntApndCfg *apndCfg = NULL;

    attr = xmlHasProp(node,(CONST xmlChar*)"type");
    if(attr == NULL){
        LOG_LOG(LOG_INT_ERR,"Appender type not exist\n");
        return LOGERR_APND_TYPE_NOT_EXSIT;
    }
    else {
        xmlChar *val = NULL;

        val = xmlNodeGetContent(attr->children);

        if(comlib_strCaseCmp((CHAR*)val, "file") == 0){
            apndType = LOGLIB_APND_TYPE_FILE;
        }
        else if(comlib_strCaseCmp((CHAR*)val, "syslog") == 0){
            apndType = LOGLIB_APND_TYPE_SYSLOG;
        }
        else if(comlib_strCaseCmp((CHAR*)val, "stdout") == 0){
            apndType = LOGLIB_APND_TYPE_STDOUT;
        }
        else {
            LOG_LOG(LOG_INT_ERR,"Invalid appender type(%s)\n",val);
            xmlFree(val);
            return LOGERR_APND_INVALID_TYPE;
        }
        xmlFree(val);
    }

    apndNode = node->xmlChildrenNode;

    apndCfg = comlib_memMalloc(sizeof(LoglibIntApndCfg));

    apndCfg->lnkNode.data = apndCfg;
    apndCfg->type = apndType;

    LOGLIB_INIT_APND_CFG(&apndCfg->apndCfg);

    while(1){
        apndNode = xmlNextElementSibling(apndNode);
        if(apndNode == NULL){
            break;
        }

        ret = comlib_strCaseCmp((CHAR*)apndNode->name,"NAME");
        if(ret == 0){
            xmlChar *val = NULL;

            val = xmlNodeGetContent(apndNode);

            apndCfg->name = (CHAR*)val;
        }

        ret = comlib_strCaseCmp((CHAR*)apndNode->name,"DISP_LEVEL");
        if(ret == 0){
            xmlChar *val = NULL;

            val = xmlNodeGetContent(apndNode);

            if(comlib_strCaseCmp((CHAR*)val,"true") == 0){
                apndCfg->apndCfg.dispBit |= LOGLIB_DISP_LVL_BIT;
            }

            xmlFree(val);
            continue;
        }

        ret = comlib_strCaseCmp((CHAR*)apndNode->name,"DISP_FILE");
        if(ret == 0){
            xmlChar *val = NULL;

            val = xmlNodeGetContent(apndNode);

            if(comlib_strCaseCmp((CHAR*)val,"true") == 0){
                apndCfg->apndCfg.dispBit |= LOGLIB_DISP_FILE_BIT;
            }

            xmlFree(val);
            continue;
        }

        ret = comlib_strCaseCmp((CHAR*)apndNode->name,"DISP_LINE");
        if(ret == 0){
            xmlChar *val = NULL;

            val = xmlNodeGetContent(apndNode);

            if(comlib_strCaseCmp((CHAR*)val,"true") == 0){
                apndCfg->apndCfg.dispBit |= LOGLIB_DISP_LINE_BIT;
            }

            xmlFree(val);
            continue;
        }

        ret = comlib_strCaseCmp((CHAR*)apndNode->name,"DISP_TIME");
        if(ret == 0){
            xmlChar *val = NULL;

            val = xmlNodeGetContent(apndNode);

            if(comlib_strCaseCmp((CHAR*)val,"true") == 0){
                apndCfg->apndCfg.dispBit |= LOGLIB_DISP_TIME_BIT;
            }

            xmlFree(val);
            continue;
        }

        ret = comlib_strCaseCmp((CHAR*)apndNode->name,"LOG_LEVEL");
        if(ret == 0){
            UINT logLvl = 0;
            xmlChar *val = NULL;

            val = xmlNodeGetContent(apndNode);

            logLvl = comlib_utilAtoi((CHAR*)val, comlib_strGetLen((CHAR*)val));

            if(logLvl == 1){
                apndCfg->apndCfg.logLvlBit |= LOGLIB_APND_DISP_ERR_LOG_BIT;
            }
            else if(logLvl == 2){
                apndCfg->apndCfg.logLvlBit |= LOGLIB_APND_DISP_NOTY_LOG_BIT;
            }
            else if(logLvl == 3){
                apndCfg->apndCfg.logLvlBit |= LOGLIB_APND_DISP_DBG_LOG_BIT;
            }

            xmlFree(val);
            continue;
        }

        switch(apndType){
            case LOGLIB_APND_TYPE_FILE:
                {
                    ret = comlib_strCaseCmp((CHAR*)apndNode->name,"LOG_PATH");
                    if(ret == 0){
                        xmlChar *val = NULL;

                        val = xmlNodeGetContent(apndNode);

                        apndCfg->apndCfg.u.file.logPath = (CHAR*)val;

                        filePathFlg = RC_TRUE;
                        continue;
                    }

                    ret = comlib_strCaseCmp((CHAR*)apndNode->name,"LOG_NAME");
                    if(ret == 0){
                        xmlChar *val = NULL;

                        val = xmlNodeGetContent(apndNode);

                        apndCfg->apndCfg.u.file.name= (CHAR*)val;

                        logNameFlg = RC_TRUE;

                        continue;
                    }

                    ret = comlib_strCaseCmp((CHAR*)apndNode->name,"MAX_LOG_SIZE");
                    if(ret == 0){
                        xmlChar *val = NULL;

                        val = xmlNodeGetContent(apndNode);

                        apndCfg->apndCfg.u.file.maxLogSize = comlib_utilAtoi((CHAR*)val, comlib_strGetLen((CHAR*)val));

                        xmlFree(val);
                        logSizeFlg = RC_TRUE;

                        continue;
                    }
                }
                break;
            case LOGLIB_APND_TYPE_SYSLOG:
                {
                    ret = comlib_strCaseCmp((CHAR*)apndNode->name,"FACILITY");
                    if(ret == 0){
                        UINT fac = 0;
                        xmlChar *val = NULL;

                        val = xmlNodeGetContent(apndNode);

                        if(comlib_strCaseCmp((CHAR*)val,"LOG_KERN") == 0)             { fac = LOG_KERN; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_USER") == 0)        { fac = LOG_USER; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_MAIL") == 0)        { fac = LOG_MAIL; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_DAEMON") == 0)      { fac = LOG_DAEMON; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_AUTH") == 0)        { fac = LOG_AUTH; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_SYSLOG") == 0)      { fac = LOG_SYSLOG; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_LPR") == 0)         { fac = LOG_LPR; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_NEWS") == 0)        { fac = LOG_NEWS; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_UUCP") == 0)        { fac = LOG_UUCP; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_CRON") == 0)        { fac = LOG_CRON; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_AUTHPRIV") == 0)    { fac = LOG_AUTHPRIV; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_FTP") == 0)         { fac = LOG_FTP; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_LOCAL0") == 0)      { fac = LOG_LOCAL0; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_LOCAL1") == 0)      { fac = LOG_LOCAL1; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_LOCAL2") == 0)      { fac = LOG_LOCAL2; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_LOCAL3") == 0)      { fac = LOG_LOCAL3; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_LOCAL4") == 0)      { fac = LOG_LOCAL4; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_LOCAL5") == 0)      { fac = LOG_LOCAL5; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_LOCAL6") == 0)      { fac = LOG_LOCAL6; }
                        else if(comlib_strCaseCmp((CHAR*)val,"LOG_LOCAL7") == 0)      { fac = LOG_LOCAL7; }
                        else {
                            LOG_LOG(LOG_INT_ERR,"Invalid fac(%s)\n",val);
                            xmlFree(val);
                            return LOGERR_INVALID_SYSLOG_FAC_TYPE;
                        }

                        apndCfg->apndCfg.u.syslog.fac = fac;

                        xmlFree(val);

                        facFlg = RC_TRUE;
                        continue;
                    }
                }
                break;
            case LOGLIB_APND_TYPE_STDOUT:
                {
                    ret = comlib_strCaseCmp((CHAR*)apndNode->name,"TYPE");
                    if(ret == 0){
                        xmlChar *val = NULL;

                        val = xmlNodeGetContent(apndNode);

                        apndCfg->apndCfg.u.stdout.type = comlib_utilAtoi((CHAR*)val, comlib_strGetLen((CHAR*)val));

                        if((apndCfg->apndCfg.u.stdout.type != LOGLIB_STDOUT_TYPE_ERR) &&
                                (apndCfg->apndCfg.u.stdout.type != LOGLIB_STDOUT_TYPE_STD)){
                            LOG_LOG(LOG_INT_ERR,"Invalid stdout type(%d)\n", apndCfg->apndCfg.u.stdout.type);
                            xmlFree(val);
                            comlib_memFree(apndCfg);
                            return LOGERR_INVALID_STDOUT_TYPE;
                        }

                        xmlFree(val);
                        stdTypeFlg = RC_TRUE;

                        continue;
                    }
                }
                break;
            default:
                {
                    LOG_LOG(LOG_INT_ERR,"Invalid appender type(%d)\n",apndType);
                    return LOGERR_APND_INVALID_TYPE;
                }
                break;
        };
    }/* end of while(1) */

    if(((apndType == LOGLIB_APND_TYPE_STDOUT) && (stdTypeFlg == RC_FALSE)) || 
            ((apndType == LOGLIB_APND_TYPE_SYSLOG) && (facFlg == RC_FALSE)) || 
            ((apndType == LOGLIB_APND_TYPE_FILE) && (filePathFlg == RC_FALSE) && 
             (logNameFlg == RC_FALSE) && (logSizeFlg == RC_FALSE))){
        LOG_LOG(LOG_INT_ERR,"Mandatory data not exist\n");
        return LOGERR_MAND_APND_DATA_NOT_EXIST;
    }

    (*rt_apndCfg) = apndCfg;

    return RC_OK;
}

FT_PRIVATE RT_RESULT cfg_loadLogCfg(xmlNodePtr node, xmlChar *name)
{
    SINT ret = RC_OK;
    ComlibLnkNode *lnkNode = NULL;
    xmlNodePtr cfgNode = NULL;
    LoglibCfg cfg;
    LoglibIntApndCfg *apndCfg = NULL;
    ComlibLnkLst apnderLL;

    LOGLIB_INIT_CFG(&cfg);
    cfg.dfltApndType = LOGLIB_APND_TYPE_NONE;

    ret = comlib_lnkLstInit(&apnderLL, ~0);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Appender linked list init failed(ret=%d)\n",ret);
        return LOGERR_APND_BUCKET_INIT_FAILED;
    }

    cfgNode = node->xmlChildrenNode;

    while(1){
        cfgNode = xmlNextElementSibling(cfgNode);
        if(cfgNode == NULL){
            break;
        }

        ret = comlib_strCaseCmp((CHAR*)cfgNode->name,"LOG_LEVEL");
        if(ret == 0){
            xmlChar *val = NULL;

            val = xmlNodeGetContent(cfgNode);

            cfg.dfltLogLvl = comlib_utilAtoi((CHAR*)val, comlib_strGetLen((CHAR*)val));

            xmlFree(val);
            continue;
        }

        ret = comlib_strCaseCmp((CHAR*)cfgNode->name,"WR_THRD_FLAG");
        if(ret == 0){
            xmlChar *val = NULL;

            val = xmlNodeGetContent(cfgNode);

            ret = comlib_strCaseCmp((CHAR*)val,"true");
            if(ret == 0){
                cfg.wrType = LOGLIB_WR_TYPE_THRD;
            }
            else {
                cfg.wrType = LOGLIB_WR_TYPE_DIR;
            }

            xmlFree(val);
            continue;
        }

        ret = comlib_strCaseCmp((CHAR*)cfgNode->name,"APPENDER");
        if(ret == 0){
            ret = cfg_loadApnd(cfgNode, &apndCfg);
            if(ret != RC_OK){
                LOG_LOG(LOG_INT_ERR,"Appender load failed(ret=%d)\n",ret);
                return ret;
            }
            ret = comlib_lnkLstInsertTail(&apnderLL, &apndCfg->lnkNode);
            if(ret != RC_OK){
                LOG_LOG(LOG_INT_ERR,"new Appender  create failed(ret=%d)\n",ret);
                comlib_memFree(apndCfg);
                return LOGERR_APNDER_INSERT_FAILED;
            }

        }
    }/* end of while(1) */

    /* load */
    ret = loglib_apiInit((CHAR*)name, &cfg);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Loglib init failed(ret=%d)\n",ret);
        return ret;
    }

    while(1){
        lnkNode = comlib_lnkLstGetFirst(&apnderLL);
        if(lnkNode == NULL){
            break;
        }

        apndCfg = lnkNode->data;

        ret = loglib_apiRegApnd((CHAR*)name, apndCfg->name, apndCfg->type, &apndCfg->apndCfg);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Appender regist failed(ret=%d)\n",ret);
            return ret;
        }

        ret = loglibInt_loadDstryApndCfg(apndCfg);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Appender destory failed(ret=%d)\n",ret);
        }

        comlib_memFree(apndCfg);
    }/* end of while(1) */

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_loadXml(CONST CHAR *cfgFile, CONST CHAR *cfgName)
{
    SINT ret = RC_OK;
    BOOL loadFlg = RC_FALSE;
    xmlAttrPtr attr = NULL;
    xmlDocPtr pDoc = NULL;
    xmlNodePtr node = NULL;
    xmlNodePtr logNode = NULL;
    xmlChar *name = NULL;

    pDoc = xmlReadFile(cfgFile, "euc-kr", XML_PARSE_RECOVER);
    if(pDoc == NULL){
        LOG_LOG(LOG_INT_ERR,"XML Parsing failed(cfg=%s)\n",cfgFile);
        return LOGERR_XML_PARSING_FAILED;
    }

    node = xmlDocGetRootElement(pDoc);

    while(1){
        ret = comlib_strCaseCmp((CHAR*)node->name,"LOG_CFG");
        if(ret == 0){
            /* find name */
            attr = xmlHasProp(node,(CONST xmlChar*)"name");
            if((attr != NULL) && (cfgName != NULL)){
                xmlChar *val = NULL;

                val = xmlNodeGetContent(attr->children);

                ret = comlib_strCaseCmp((CHAR*)val, cfgName);
                if(ret != 0){
                    xmlFree(val);
                    continue;
                }
                xmlFree(val);
            }/* end of if(attr != NULL) */
            else if(attr != NULL){
                if(cfgName == NULL){
                    continue;
                }
            }

            /* find child */
            logNode = node->xmlChildrenNode;
            while(1){
                logNode = xmlNextElementSibling(logNode);
                if(logNode == NULL){
                    break;
                }

                name = NULL;

                attr = xmlHasProp(logNode,(CONST xmlChar*)"name");
                if(attr != NULL){
                    xmlChar *val = NULL;

                    val = xmlNodeGetContent(attr->children);

                    name = val;
                }/* end of if(attr != NULL) */

                ret = cfg_loadLogCfg(logNode, name);
                if(ret != RC_OK){
                    LOG_LOG(LOG_INT_ERR,"LOG XML Parsing failed(ret=%d)\n",ret);
                    if(name != NULL){
                        xmlFree(name);
                    }
                    return ret;
                }

                if(name != NULL){
                    xmlFree(name);
                }
                loadFlg = RC_TRUE;
            }/* end of while(1) */

            break;
        }/* end of if(ret == 0) */

        node = xmlNextElementSibling(node);
        if(node == NULL){
            break;
        }
    }/* end of while(1) */

    xmlFreeDoc(pDoc);

    if(loadFlg == RC_FALSE){
        LOG_LOG(LOG_INT_ERR,"Log xml load failed(LOG_CFG not exist)\n");
        return LOGERR_LOG_CFG_NOT_EXIST;
    }

    return RC_OK;
}

#endif /* LOGLIB_XML_CONFIG_DISABLE */
