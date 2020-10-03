#!/bin/bash
# Created By [CTCGFW]Project OpenWRT
# https://github.com/project-openwrt

header="const cache = require('../cache')
const insure = require('./insure')
const select = require('./select')
const request = require('../request')

const headers = {
        'origin': 'http://y.qq.com/',
        'referer': 'http://y.qq.com/',"

body="const playable = song => {
	const switchFlag = song['switch'].toString(2).split('')
	switchFlag.pop()
	switchFlag.reverse()
	const playFlag = switchFlag[0]
	const tryFlag = switchFlag[13]
	return ((playFlag == 1) || ((playFlag == 1) && (tryFlag == 1)))
}

const format = song => ({
	id: {song: song.mid, file: song.file.media_mid},
	name: song.name,
	duration: song.interval * 1000,
	album: {id: song.album.mid, name: song.album.name},
	artists: song.singer.map(({mid, name}) => ({id: mid, name}))
})

const search = info => {
	const url =
		'https://c.y.qq.com/soso/fcgi-bin/client_search_cp?' +
		'ct=24&qqmusic_ver=1298&new_json=1&remoteplace=txt.yqq.center&' +
		't=0&aggr=1&cr=1&catZhida=1&lossless=0&flag_qc=0&p=1&n=20&w=' +
		encodeURIComponent(info.keyword) + '&' +
		'g_tk=5381&jsonpCallback=MusicJsonCallback10005317669353331&loginUin=0&hostUin=0&' +
		'format=jsonp&inCharset=utf8&outCharset=utf-8&notice=0&platform=yqq&needNewCode=0'

	return request('GET', url)
	.then(response => response.jsonp())
	.then(jsonBody => {
		const list = jsonBody.data.song.list.map(format)
		const matched = select(list, info)
		return matched ? matched.id : Promise.reject()
	})
}"

if [ "$1" == "local" ]
then echo -e "// local_mode
${header}
	'cookie': process.env.QQ_COOKIE || null // 'uin=; qm_keyst=',
}

${body}

const single = (id, format) => {
	const uin = ((headers.cookie || '').match(/uin=(\d+)/) || [])[1] || '0'

	const concatenate = vkey => {
		if (!vkey) return Promise.reject()
		const host = ['streamoc.music.tc.qq.com', 'mobileoc.music.tc.qq.com', 'isure.stream.qqmusic.qq.com', 'dl.stream.qqmusic.qq.com', 'aqqmusic.tc.qq.com/amobile.music.tc.qq.com'][3]
		return \`http://\${host}/\${format.join(id.file)}?vkey=\${vkey}&uin=0&fromtag=8&guid=7332953645\`
	}

	const url =
		'https://u.y.qq.com/cgi-bin/musicu.fcg?data=' +
		encodeURIComponent(JSON.stringify({
			req_0: {
				module: 'vkey.GetVkeyServer',
				method: 'CgiGetVkey',
				param: {
					guid: '7332953645',
					loginflag: 1,
					filename: [format.join(id.file)],
					songmid: [id.song],
					songtype: [0],
					uin,
					platform: '20'
				}
			}
		}))

	return request('GET', url, headers)
	.then(response => response.json())
	.then(jsonBody => {
		const { sip, midurlinfo } = jsonBody.req_0.data
		return midurlinfo[0].purl ? sip[0] + midurlinfo[0].purl : Promise.reject()
	})
}

const track = id => {
	id.key = id.file
	return Promise.all(
		[['F000', '.flac'], ['M800', '.mp3'], ['M500', '.mp3']].slice((headers.cookie || typeof(window) !== 'undefined') ? (select.ENABLE_FLAC ? 0 : 1) : 2)
		.map(format => single(id, format).catch(() => null))
	)
	.then(result => result.find(url => url) || Promise.reject())
	.catch(() => insure().qq.track(id))
}

const check = info => cache(search, info).then(track)

module.exports = {check, track}"
elif [ "$1" == "remote" ]
then [ "$2" == "1" ] && { flac_quality="'flac', "; loop_times="4"; } || loop_times="3"
echo -e "// remote_mode
${header}
	'cookie': null
}

${body}

const track = id => {
	const typeObj = [${flac_quality}'320', '128', 'm4a']

	let i = 0
	while (i < ${loop_times}) {
		type = typeObj[i]
		let url =
			'https://api.qq.jsososo.com/song/url?id=' +
			id.song + '&mediaId=' + id.file + '&type=' + \`\${type}\`

		return request('GET', url)
		.then(response => response.json())
		.then(jsonBody => {
			let res = jsonBody.result
			if (res === 100) {
				let songUrl = jsonBody.data
				return songUrl
			} else if (i === $[${loop_times}-1]) {
				return Promise.reject()
			} else {
				return
			}
		})
		.catch(() => insure().qq.track(id))
		i++
	}
}

const check = info => cache(search, info).then(track)

module.exports = {check, track}"
else echo -e "Usage: $0 [local|remote] (1)"
fi
