#pragma once
#include <Windows.h>
#include <exception>
#include <string>

#include "Interface.hpp"
#include "Structure.hpp"
#include "Buffer.hpp"
#include "Channel.hpp"

namespace asio
{
	/**
	* ドライバのインスタンスに生成失敗すると呼ばれる
	*/
	class CantCreateInstance : std::exception
	{
	public:
		CantCreateInstance(const std::string& message)
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

		ChannelManager* channelManager;
		BufferManager* bufferManager;

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

			iasio->init(systemHandle);

			// 名前とドライバのバージョンだけ取得
			char buffer[360];
			iasio->getDriverName(buffer);
			driverName = buffer;
			driverVersion = iasio->getDriverVersion();

			channelManager = new ChannelManager(iasio);
			bufferManager = new BufferManager(iasio);
		}

		/**
		* コピー禁止
		*/
		Driver(const Driver& hoge) {}

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
		inline const std::vector<Channel>& InputChannels() const { return channelManager->Inputs(); }
		
		/**
		* 出力チャンネルの配列を返す
		* @return 出力チャンネルの配列
		*/
		inline const std::vector<Channel>& OutputChannels() const { return channelManager->Outputs(); }

		/**
		* 添字から入力チャンネルを返す
		* @return 入力チャンネル
		*/
		inline const Channel& InputChannel(const long i) const { return channelManager->Inputs(i); }

		/**
		* 添字から出力チャンネルを返す
		* @return 出力チャンネル
		*/
		inline const Channel& OutputChannel(const long i) const { return channelManager->Outputs(i); }


	public:		// バッファ周り

		/**
		* ASIOのバッファの設定を取得
		* @return バッファの現在の設定
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
		* @note この関数を使うとドライバ側で設定されているバッファサイズを利用します
		*/
		const BufferArray& CreateBuffer(const Channel& channel)
		{
			ASIOCallbacks callback = Buffer::CreateCallbacks();

			bufferManager->AddChannel(channel);
			auto retval = bufferManager->CreateBuffer(GetBufferPreference(), &callback);
			bufferManager->ClearChannel();
			
			return retval;
		}

		/**
		* バッファリング開始
		*/
		const void Start() const
		{
			iasio->start();
		}

		/**
		* バッファリング一時停止
		*/
		const void Stop() const
		{
			iasio->stop();
		}

	public:
		/**
		* シングルトンメソッド
		*/
		static Driver& Get(const CLSID& clsid)
		{
			if (Driver::driver == nullptr)
				Driver::driver = std::make_shared<Driver>(clsid);
		}

		/**
		* ドライバの解放など
		*/
		virtual ~Driver()
		{
			iasio->disposeBuffers();
			iasio->Release();

			delete channelManager;
			delete bufferManager;
		}
	};

	std::shared_ptr<Driver> Driver::driver = std::make_shared<Driver>(nullptr);

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