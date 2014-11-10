#pragma once
#include <Windows.h>
#include <exception>
#include <string>
#include <memory>

#include "Exception.hpp"
#include "Registory.hpp"
#include "SDK.hpp"
#include "Interface.hpp"

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

	private:
		
		/**
		* @params[in] clsid ロードしたいCLSID
		* @params[in] subkey レジストリの位置など
		*/
		Driver(const CLSID& clsid, const SubKey& subkey)
			: iasio(clsid, subkey) { }


	public:
		const std::string& Name() const { return iasio.Name(); }	//!< ドライバ名を返す
		const long& Version() const { return iasio.Version(); }		//!< ドライバのバージョンを返す
		IASIO* Interface() { return iasio.IASIO(); }					//!< ASIOのインターフェースを返す

	public:
		/**
		* ドライバの初期化
		* @params[in] subkey ASIOドライバのSubkey
		* @return subkeyで初期化されたドライバのインスタンス
		* @note 以前に生成されたドライバは破棄される
		*/
		static Driver& Init(const SubKey& subkey)
		{
			auto clsid = Registory::GetCLSID(subkey.registoryPath);
			Driver::driver.reset(new Driver(clsid, subkey), [](Driver *p) { delete p; });
			return *Driver::driver;
		}


	private:
		// 順番の都合上，ここにいる
		template <typename T>
		static Driver& InitX(const T& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}


	public:
		/**
		* ドライバの初期化
		* @params[in] asioDriverName ASIOドライバの名前を検索して，それで初期化を行う
		* @return asioDriverNameで初期化されたドライバのインスタンス
		* @note 見つからない場合はたぶん落ちる
		*/
		static Driver& Init(const std::string& asioDriverName) { return InitX(asioDriverName); }
		

		/**
		* ドライバの初期化
		* @params[in] asioDriverName ASIOドライバの名前を検索して，それで初期化を行う
		* @return asioDriverNameで初期化されたドライバのインスタンス
		* @note 見つからない場合はたぶん落ちる
		*/
		static Driver& Init(const std::wstring& asioDriverName)	{ return InitX(asioDriverName);	}


		/**
		* ドライバの取得
		*/
		static Driver& Get() { return *driver; }


		/**
		* ドライバの解放など
		*/
		~Driver() {	}
	};

	std::shared_ptr<Driver> Driver::driver;
}