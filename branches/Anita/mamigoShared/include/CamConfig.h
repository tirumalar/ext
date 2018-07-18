/*
 * CamConfig.h
 *
 *  Created on: Jan 10, 2011
 *      Author: developer1
 */

#ifndef CAMCONFIG_H_
#define CAMCONFIG_H_

struct camconfig;
class Configuration;


class CameraConfig {
public:
	CameraConfig();
	virtual ~CameraConfig();
	void configure(Configuration *pCfg, camconfig *camCfg);
	void map_uservals_to_registers(Configuration *pCfg,camconfig* newconfig);
	void Init(){ CameraParam = Getnewcamconfig();}
	camconfig* Get_config(){ return CameraParam;}
	camconfig* Getnewcamconfig();
	struct camconfig *CameraParam;
};

#endif /* CAMCONFIG_H_ */
