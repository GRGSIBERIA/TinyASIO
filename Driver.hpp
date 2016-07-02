/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

TinyASIO is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

TinyASIO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TinyASIO.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

#pragma once
#include <Windows.h>
#include <exception>
#include <string>
#include <memory>

#include "Exception.hpp"
#include "Registry.hpp"
#include "SDK.hpp"
#include "Interface.hpp"
#include "Channel.hpp"

namespace asio
{
	/**
	* ASIOドライバのインターフェースのラッパクラス
	*/
	class Driver
	{
	private:
		static std::shared_ptr<Driver> driver;	// シングルトン

		Interface iasio;
		std::shared_ptr<ChannelManager> channelManager;

	private:
		
		/**
		* @param[in] clsid ロードしたいCLSID
		* @param[in] subkey レジストリの位置など
		*/
		Driver(const CLSID& clsid, const SubKey& subkey)
			: iasio(clsid, subkey)
		{
			channelManager.reset(new asio::ChannelManager(iasio.IASIO()), [](asio::ChannelManager* p) { delete p; });
		}


	public:
		const std::string& Name() const { return iasio.Name(); }	//!< ドライバ名を返す
		const long& Version() const { return iasio.Version(); }		//!< ドライバのバージョンを返す

		IASIO* Interface() { return iasio.IASIO(); }				//!< ASIOのインターフェースを返す

	public:
		/**
		* ドライバの初期化
		* @param[in] subkey ASIOドライバのSubkey
		* @return subkeyで初期化されたドライバのインスタンス
		* @note 以前に生成されたドライバは破棄される
		*/
		static Driver& Init(const SubKey& subkey)
		{
			auto clsid = Registry::GetCLSID(subkey.registryPath);
			Driver::driver.reset(new Driver(clsid, subkey), [](Driver *p) { delete p; });
			return *Driver::driver;
		}

		const ChannelManager& ChannelManager() const { return *channelManager; }


	private:
		// 順番の都合上，ここにいる
		template <typename T>
		static Driver& InitX(const T& asioDriverName)
		{
			auto asioList = asio::Registry::GetAsioDriverPathes();
			auto asioRegistry = asioList.Find(asioDriverName);
			return Init(asioRegistry);
		}


	public:
		/**
		* ドライバの初期化
		* @param[in] asioDriverName ASIOドライバの名前を検索して，それで初期化を行う
		* @return asioDriverNameで初期化されたドライバのインスタンス
		* @note 見つからない場合はたぶん落ちる
		*/
		static Driver& Init(const std::string& asioDriverName) { return InitX(asioDriverName); }
		

		/**
		* ドライバの初期化
		* @param[in] asioDriverName ASIOドライバの名前を検索して，それで初期化を行う
		* @return asioDriverNameで初期化されたドライバのインスタンス
		* @note 見つからない場合はたぶん落ちる
		*/
		static Driver& Init(const std::wstring& asioDriverName)	{ return InitX(asioDriverName);	}


		/**
		* ドライバの取得
		*/
		static Driver& Get() { 
			if (driver == nullptr)
				throw std::exception("ドライバがnullです");
			return *driver; 
		}

		/**
		* ドライバの解放
		*/
		static void Dispose()
		{
			// driver.reset();
		}

		/**
		* ドライバの解放など
		*/
		~Driver() 
		{
			// iasio.Release();
		}

		const std::vector<InputChannel>& InputChannels() const { return channelManager->Inputs(); }			//!< 入力チャンネルを返す
		const std::vector<OutputChannel>& OutputChannels() const { return channelManager->Outputs(); }		//!< 出力チャンネルを返す
		const InputChannel& InputChannels(const long i) const { return channelManager->Inputs(i); }			//!< 添字iに対応した入力チャンネルを返す
		const OutputChannel& OutputChannels(const long i) const { return channelManager->Outputs(i); }		//!< 添字iに対応して出力チャンネルを返す
	};

	std::shared_ptr<Driver> Driver::driver;
}