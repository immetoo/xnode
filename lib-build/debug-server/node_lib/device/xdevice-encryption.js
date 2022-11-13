var winston         = require('winston');
var log             = winston.loggers.get('main');

var delta = 0x9E3779B9;
var XXTEA_NUM_ROUNDS = 32;

//var xxxteaEncrypt = function(unsigned long v[2]) {
//	unsigned int i;
//	unsigned long v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
//	for (i=0; i < XXTEA_NUM_ROUNDS; i++) {
//		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + XBHardware.config.net_key.u32[sum & 3]);
//		sum += delta;
//		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + XBHardware.config.net_key.u32[(sum>>11) & 3]);
//	}
//	v[0]=v0; v[1]=v1;
//}

var xxxteaDecrypt = function(vArray,keyBuff) {
	log.debug('XDeviceEncryption.xxxteaDecrypt vArray='+JSON.stringify(vArray)+' keyBuff='+JSON.stringify(keyBuff));
	var v0=vArray.v0;
	var v1=vArray.v1;
	
	log.debug('XDeviceEncryption.xxxteaDecrypt start v0='+v0.toString(16)+' v1='+v1.toString(16));
	
	var delta=0x9E3779B9
	var sum=delta*XXTEA_NUM_ROUNDS;
	for (var i=0; i < XXTEA_NUM_ROUNDS; i++) {
		v1 -= (((v0 << 4) ^ (v0 >>> 5)) + v0) ^ (sum + keyBuff.readUInt32BE((sum>>>11) & 3));
		var v1Org = v1;
		
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >>> 5)) + v1) ^ (sum + keyBuff.readUInt32BE(sum & 3));
		
		//log.debug('XDeviceEncryption.xxxteaDecrypt v0='+v0.toString(16)+' v1='+v1.toString(16));
		
		if (v0 < 0) { v0 = Math.pow(2,32) + v0; }
		if (v0 > Math.pow(2,32)) { v0 = v0 - Math.pow(2,32); }
		
		if (v1 < 0) { v1 = Math.pow(2,32) + v1; }
		if (v1 > Math.pow(2,32)) { v1 = v1 - Math.pow(2,32); }
	}
	
	//v0 = v0 & 0xFFFFFFFF;
	//v1 = v1 & 0xFFFFFFFF;
	
	
	
	var result = {v0: v0,v1: v1};
	log.debug('XDeviceEncryption.xxxteaDecrypt result='+JSON.stringify(result));
	return result;
}

// public static method encrypt
exports.xxteaEncrypt = function(string, key) {
	log.debug('XDeviceEncryption.xxteaEncrypt key='+key+' data='+string);
	if (string.length == 0) {
		return('');
	}
	
	//var v = strToLongs(Utf8.encode(string));
	//var k = strToLongs(Utf8.encode(key).slice(0,16));
	var v = buffToLongs(new Buffer(string, 'hex'));
	var k = buffToLongs(new Buffer(key, 'hex')); 
	
	//if (v.length <= 1) v[1] = 0;  // algorithm doesn't work for n<2 so fudge by adding a null
	var n = v.length;
	
	// ---- <TEA coding> ---- 
	
	var z = v[n-1], y = v[0], delta = 0x9E3779B9;
	var mx, e, q = Math.floor(6 + 52/n), sum = 0;
	
	while (q-- > 0) {  // 6 + 52/n operations gives between 6 & 32 mixes on each word
		sum += delta;
		e = sum>>>2 & 3;
		for (var p = 0; p < n; p++) {
			y = v[(p+1)%n];
			mx = (z>>>5 ^ y<<2) + (y>>>3 ^ z<<4) ^ (sum^y) + (k[p&3 ^ e] ^ z);
			z = v[p] += mx;
		}
	}
	
	// ---- </TEA> ----
	
	var buff = longsToBuff(v);
	return buff.toString('ascii');
}

exports.xxteaDecrypt = function(data, key) {
	log.debug('XDeviceEncryption.xxteaDecrypt key='+key+' data='+data);
	if (data.length == 0) {
		return('');
	}
	var d = new Buffer(data, 'hex');
	var k = new Buffer(key, 'hex');
	var n = d.length;
	var r = new Buffer(n);
	
	log.debug('XDeviceEncryption.xxteaDecrypt dataBuffer='+d.toString('hex'));
	
	// ---- <TEA decoding> ---- 
	
	for (var i=0;i<n/4-1;i=i+2) {
		//log.debug('XDeviceEncryption.xxteaDecrypt n='+n+' i='+i+' c='+(i*4)+' cc='+(i*4+1)+' r='+d.readUInt32LE(i*4).toString(16)+' r1='+d.readUInt32BE(i*4).toString(16));
		var ed = {v0: d.readUInt32BE(i*4),v1: d.readUInt32BE((i+1)*4)};
		var dd = xxxteaDecrypt(ed,k);
		r.writeUInt32BE(dd.v0,i*4);
		r.writeUInt32BE(dd.v1,(i+1)*4);
	}
	return r.toString('ascii');
	
	//var plaintext = longsToStr(v); 
	//plaintext = plaintext.replace(/\0+$/,'');
	//return plaintext;
	//return new Buffer(plaintext,'utf8').toString('ascii');
	
	//var plaintext = longsToStr(v);
	//plaintext = plaintext.replace(/\0+$/,'');// strip trailing null chars resulting from filling 4-char blocks:
	//return Utf8.decode(plaintext);
}
