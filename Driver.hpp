#pragma once
#include <Windows.h>
#include <exception>
#include <string>
#include <memory>

#include "Registory.hpp"
#include "Interface.hpp"
#include "Structure.hpp"
#include "Channel.hpp"
#include "BufferManager.hpp"

namespace asio
{
	/**
	* ドライバのインスタンスに生成失敗すると呼ばれる
	*/
	class CantCreateInstance : public std::exception
	{
	public:
		CantCreateInstance(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* 二回以上初期化されたりなどで呼び出される
	*/
	class OverTwiceCallException : public std::exception
	{
	public:
		OverTwiceCallException(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* ドライバのハンドルを取得できなかった
	*/
	class CantHandlingASIODriver : public std::exception
	{
	public:
		CantHandlingASIODriver(const std::string& message)
			: exception(message.c_str()) {}
	};

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

		std::shared_ptr<ChannelManager> channelManager;
		std::shared_ptr<BufferManager> bufferManager;

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

		/**
		* @params[in] clsid ロードしたいCLSID
		*/
		Driver(const CLSID& clsid)
		{
			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");

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

			channelManager = std::shared_ptr<ChannelManager>(new ChannelManager(iasio));
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(iasio));
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

		/**
		* ドライバのインターフェースを返す
		*/
		const IASIO& Interface() const { return *iasio; }


	public:		// チャンネル周り

		/**
		* 入力チャンネルの配列を返す
		* @return 入力チャンネルの配列
		*/
		inline const std::vector<InputChannel>& InputChannels() const { return channelManager->Inputs(); }
		
		/**
		* 出力チャンネルの配列を返す
		* @return 出力チャンネルの配列
		*/
		inline const std::vector<OutputChannel>& OutputChannels() const { return channelManager->Outputs(); }

		/**
		* チャンネルを追加
		*/
		inline const void AddChannel(const Channel& channel) { bufferManager->AddChannel(channel); }

		/**
		* 登録したチャンネルを削除
		*/
		inline const void ClearChannels() { bufferManager->ClearChannel(); }


	public:		// バッファ周り

		/**
		* ASIOのバッファの設定を取得
		* @return バッファの現在の設定
		* @note BufferPreferenceの値を変更してCreateBufferへ渡す
		* @note 必ずしも信用できる値を取得できるとは限らない
		*/
		BufferPreference GetBufferPreference() const
		{
			BufferPreference buf;
			ErrorCheck(iasio->getBufferSize(&buf.minSize, &buf.maxSize, &buf.preferredSize, &buf.granularity));
			return buf;
		}

		/**
		* バッファの生成
		* @params[in] sample サンプリング方法
		* @params[in] bufferPref バッファの設定
		* @return バッファのコントローラ
		*/
		const BufferController& CreateBuffer(const pack::Sample& sample, const BufferPreference& bufferPref)
		{
			ASIOCallbacks callback = callback::CallbackManager::CreateCallbacks();

			//if (bufferManager != nullptr)		// バッファを重複して利用させない作戦
			//	delete bufferManager;
			//bufferManager = new BufferManager(iasio);

			auto& bufferCtrl = bufferManager->CreateBuffer(bufferPref, sample.ToSampleType(), &callback);

			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(iasio));
			
			return bufferCtrl;
		}

		/**
		* バッファを生成する
		* @params[in] sample サンプリング方法
		* @return バッファのコントローラ
		* @note この関数を使うとドライバ側で設定されているバッファサイズを利用します
		*/
		const BufferController& CreateBuffer(const pack::Sample& sample)
		{
			return CreateBuffer(sample, GetBufferPreference());
		}

	public:
		/**
		* ドライバの初期化
		* @params[in] subkey ASIOドライバのSubkey
		* @note 以前に生成されたドライバは破棄される
		*/
		static Driver& Init(const SubKey& subkey)
		{
			auto clsid = Registory::GetCLSID(subkey.registoryPath);
			Driver::driver.reset(new Driver(clsid), [](Driver *p) { delete p; });
			return *Driver::driver;
		}

		/**
		* ドライバインスタンスの取得
		* @return ドライバインスタンス
		*/
		static Driver& Get()
		{
			return *Driver::driver;
		}

		/**
		* ドライバの解放など
		*/
		virtual ~Driver()
		{
			iasio->Release();
		}
	};

	std::shared_ptr<Driver> Driver::driver;

	// メモ
	//virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) = 0;
	//virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
	//	long bufferSize, ASIOCallbacks *callbacks) = 0;
	//virtual ASIOError disposeBuffers() = 0;
	//virtual ASIOError controlPanel() = 0;
	//virtual ASIOError future(long selector, void *opt) = 0;
	//virtual ASIOError outputReady() = 0;

	// 必要なのかどうかわからないので保留
	//virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources) = 0;
	//virtual ASIOError setClockSource(long reference) = 0;
	//virtual void getErrorMessage(char *string) = 0;
}