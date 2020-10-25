#!/bin/bash
log_file="/tmp/unblockneteasemusic.log"
json_file="/tmp/unblockneteasemusic_latest.json"
if [ "$(uci get unblockneteasemusic.@unblockneteasemusic[0].auto_update)" == "1" ]; then
    echo "$(date -R) check latest version ..." >>"${log_file}"
    curl -s -o "${json_file}" https://api.github.com/repos/cnsilvan/UnblockNeteaseMusic/releases/latest
    if [ $? -ne 0 ]; then
       echo "$(date -R) curl api.github.com failed" >>"${log_file}"
       exit 1
    fi
    currentTagCMD="$(UnblockNeteaseMusic -v | grep Version | awk '{print $2}')"
    currentRuntimeCMD="$(UnblockNeteaseMusic -v | grep runtime | awk -F\( '{print $2}' | awk '{print $3,$4}' | sed -E 's/\)//g'| sed 's/[ \t]*$//g')"
    latestTagCMD="$(cat ${json_file} | grep '\"tag_name\":' | sed -E 's/.*\"([^\"]+)\".*/\1/')"
    GOOSS="$(echo $currentRuntimeCMD | awk '{print $1}')"
    GOOS="linux"
    GOARCH="amd64"
    suffix="$(echo $currentRuntimeCMD | awk '{print $2}')"
    if [ ! -n "$suffix" ]; then
        suffix=".zip"
    fi
    if [ "$suffix" == "hardfloat" ]; then
        suffix=".zip"
    fi
    if [ -n "$(echo $GOOSS | awk -F/ '{print $1}')" ]; then
        GOOS="$(echo $GOOSS | awk -F/ '{print $1}')"
    fi
    if [ -n "$(echo $GOOSS | awk -F/ '{print $2}')" ]; then
        GOARCH="$(echo $GOOSS | awk -F/ '{print $2}')"
    fi
    downloadUrl="$(cat ${json_file} | grep '\"browser_download_url\":' | grep ${GOOS} | grep ${GOARCH} | grep ${suffix} | sed -E 's/.*\"([^\"]+)\".*/\1/')"
    if [ ! -n "${downloadUrl}" ]; then
       echo "$(date -R) not found ${currentRuntimeCMD} on GitHub,please go to https://github.com/cnsilvan/luci-app-unblockneteasemusic/issues to open a issue " >>"${log_file}"
       exit 1
    fi
    if [ "${currentTagCMD}" == "${latestTagCMD}" ]; then
        echo "$(date -R) current version: ${currentTagCMD}(${currentRuntimeCMD}) is the latest version" >>"${log_file}"
    else
        echo "$(date -R) start downloading the latest version[ ${currentTagCMD}(${currentRuntimeCMD}) >> ${latestTagCMD}(${currentRuntimeCMD}) ]..." >>"${log_file}"
        echo "$(date -R) ${downloadUrl}" >>"${log_file}"
        curl -LJO ${downloadUrl}
        if [ $? -eq 0 ]; then
            echo "$(date -R) download successful" >>"${log_file}"
            unzip $(find . -type f -name "*UnblockNeteaseMusic*.zip") -d ./UnblockNeteaseMusic/
            rm -f $(find . -type f -name "*UnblockNeteaseMusic*.zip")
            chmod +x ./UnblockNeteaseMusic/UnblockNeteaseMusic
            if [ -n "$(./UnblockNeteaseMusic/UnblockNeteaseMusic -v | grep Version | awk '{print $2}')" ]; then
                mv ./UnblockNeteaseMusic/UnblockNeteaseMusic /usr/bin/
                rm -rf ./UnblockNeteaseMusic/
                echo "$(date -R) update successful" >>"${log_file}"
                /etc/init.d/unblockneteasemusic restart
            else
                rm -rf ./UnblockNeteaseMusic/
                echo "$(date -R) update failed. please check if the downloaded version is correct" >>"${log_file}"
            fi
        else
            echo "$(date -R) download failed" >>"${log_file}"

        fi

    fi

fi
