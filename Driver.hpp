#pragma once
#include <Windows.h>
#include <exception>
#include <string>
#include <memory>

#include "Exception.hpp"
#include "Registory.hpp"
#include "SDK.hpp"

namespace asio
{
	/**
	* ASIOドライバのインターフェースのラッパクラス
	*/
	class Driver
	{
	private:
		static std::shared_ptr<Driver> driver;	// シングルトン変数

		IASIO *iasio;			// インターフェースへのポインタ
		void *systemHandle;		// 謎のシステムハンドル

		std::string driverName;
		long driverVersion;
		SubKey subkey;
		ASIOCallbacks callback;


	private:
		ASIOCallbacks InitNullCallbacks()
		{
			ASIOCallbacks callback;
			callback.asioMessage = NULL;
			callback.bufferSwitch = NULL;
			callback.bufferSwitchTimeInfo = NULL;
			callback.sampleRateDidChange = NULL;
			return callback;
		}


		void RetryCreateInstance(const CLSID& clsid, const SubKey& subkey)
		{
			// デフォルトだとThreadingModelがSTAなので，STA/MTA（Both）に変更して再試行する
			if (Registory::ChangeTheadingModel(subkey) != ERROR_SUCCESS)
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");

			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");
		}


		/**
		* @params[in] clsid ロードしたいCLSID
		* @params[in] subkey レジストリの位置など
		*/
		Driver(const CLSID& clsid, const SubKey& subkey)
			: subkey(subkey)
		{
			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				RetryCreateInstance(clsid, subkey);

			try
			{
				iasio->init(systemHandle);
			}
			catch (...)
			{
				throw CantHandlingASIODriver("ドライバのハンドルの初期化に失敗しました");
			}

			// 名前とドライバのバージョンだけ取得
			char buffer[360];
			iasio->getDriverName(buffer);
			driverName = buffer;
			driverVersion = iasio->getDriverVersion();
		}

		

	public:

		/**
		* ドライバ名を返す
		*/
		const std::string& Name() const { return driverName; }

		/**
		* ドライバのバージョンを返す
		*/
		const long& Version() const { return driverVersion; }


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


		/**
		* ドライバの初期化
		* @params[in] asioDriverName ASIOドライバの名前を検索して，それで初期化を行う
		* @return asioDriverNameで初期化されたドライバのインスタンス
		* @note 見つからない場合はたぶん落ちる
		*/
		static Driver& Init(const std::string& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}
		

		/**
		* ドライバの初期化
		* @params[in] asioDriverName ASIOドライバの名前を検索して，それで初期化を行う
		* @return asioDriverNameで初期化されたドライバのインスタンス
		* @note 見つからない場合はたぶん落ちる
		*/
		static Driver& Init(const std::wstring& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}


		/**
		* ドライバの解放など
		*/
		virtual ~Driver()
		{
			// ドライバが解放された状態らしく，あまり意味を成さない
			//ErrorCheck(iasio->disposeBuffers());
			//ErrorCheck(iasio->Release());
			//CloseHandle(hMutex);	// ミューテックスのハンドルを開放する
		}
	};

	std::shared_ptr<Driver> Driver::driver;
}