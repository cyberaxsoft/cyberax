#include <glib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include "driver.h"
#include "logger.h"
#include "config.h"
#include "shtrih.h"
#include "driver_state.h"

guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
}

guint8 prepare_enq_frame(guint8* buffer)
{
	guint8 pos = 0;

	buffer[pos++] = ctrl_ENQ;

	return pos;
}

guint8 prepare_nak_frame(guint8* buffer)
{
	guint8 pos = 0;

	buffer[pos++] = ctrl_NAK;

	return pos;
}

guint8 prepare_ack_frame(guint8* buffer)
{
	guint8 pos = 0;

	buffer[pos++] = ctrl_ACK;

	return pos;
}

guint8 calc_crc(guint8* buffer, guint8 length)
{
	guint8 result = 0;

	for (guint8 i = 1; i < length; i++)
	{
		result^=buffer[i];
	}

	return result;
}

guint8 prepare_frame(guint8* buffer, ShtrihCommand command, guint32 password, guint8* data, guint8 data_length)
{
	guint8 pos = 0;

	buffer[pos++] = ctrl_STX;
	buffer[pos++] = 0; //length
	buffer[pos++] = command; //command

	buffer[pos++] = password & 0xFF;
	buffer[pos++] = (password >> 8) & 0xFF;
	buffer[pos++] = (password >> 16) & 0xFF;
	buffer[pos++] = (password >> 24) & 0xFF;

	if (data_length > 0 && data!=NULL && (pos+data_length) < WRITE_BUFFER_SIZE)
	{
		memcpy(&buffer[pos], data, data_length);
		pos+=data_length;
	}


	buffer[LENGTH_OFFSET] = pos - 2;

	buffer[pos++] = calc_crc(buffer, pos);

	return pos;



}

guint8 prepare_get_state_frame(guint8* buffer)
{
	return prepare_frame(buffer, sc_GetExState, OPERATOR_PASSWORD, NULL, 0);
}

const gchar* command_to_str(ShtrihCommand command)
{
	switch(command)
	{
		case sc_GettDump: 				return "Запрос дампа";
		case sc_GetData: 				return "Запрос данных";
		case sc_DataInterrupt: 			return "Прерывание выдачи данных";
		case sc_GetState: 				return "Короткий запрос состояния";
		case sc_GetExState: 			return "Запрос состояния ККТ";
		case sc_PrintBoldText: 			return "Печать жирной строки (шрифт 2)";
		case sc_Beep: 				 	return "Гудок";
		case sc_SetExchangeParam: 		return "Установка параметров обмена";
		case sc_ReadExchangeParam: 		return "Чтение параметров обмена";
		case sc_HardwareReset: 			return "Технологическое обнуление";
		case sc_PrintText: 				return "Печать стандартной строки (шрифт 1)";
		case sc_PrintHeader: 			return "Печать заголовка документа";
		case sc_TextPrint: 				return "Тестовый прогон";
		case sc_GetMoneyReg: 			return "Запрос денежного регистра";
		case sc_GetOperationReg: 		return "Запрос операционного регистра";
		case sc_SaveTable: 				return "Запись таблицы";
		case sc_ReadTable: 				return "Чтение таблицы";
		case sc_SetTime: 				return "Программирование времени";
		case sc_SetDate: 				return "Программирование даты";
		case sc_ConfirmDate: 			return "Подтверждение программирования даты";
		case sc_TablesInit: 			return "Инициализация таблиц начальными значениями";
		case sc_Cut: 					return "Отрезка чека";
		case sc_GetFontParam: 			return "Прочитать параметры шрифта";
		case sc_FullRegisterClear: 		return "Общее гашение";
		case sc_OpenDrawer: 			return "Открыть денежный ящик";
		case sc_FeedPaper: 				return "Протяжка";
		case sc_InterruptPrintTest: 	return "Прерывание тестового прогона";
		case sc_GetOperationRegisters: 	return "Снятие показаний операционных регистров";
		case sc_GetTableStructure: 		return "Запрос структуры таблицы";
		case sc_GetRecordStructure: 	return "Запрос структуры поля";
		case sc_PrintTextFont: 			return "Печать строки данным шрифтом";
		case sc_XReport: 				return "Суточный отчет без гашения";
		case sc_ZReport: 				return "Суточный отчет с гашением";
		case sc_PrintImage512: 			return "Печать графики-512 с масштабированием1";
		case sc_LoadImage512: 			return "Загрузка графики-5121";
		case sc_PrintImage: 			return "Печать графики с масштабированием";
		case sc_Income: 				return "Внесение";
		case sc_Output: 				return "Выплата";
		case sc_PrintCliche: 			return "Печать клише";
		case sc_CloseDocument: 			return "Конец документа";
		case sc_PrintPromoText: 		return "Печать рекламного текста";
		case sc_GetErrorDescription: 	return "Возврат названия ошибки";
		case sc_Sale: 					return "Продажа";
		case sc_Buy: 					return "Покупка";
		case sc_ReturnSale: 			return "Возврат продажи";
		case sc_ReturnBuy: 				return "Возврат покупки";
		case sc_CloseReceipt: 			return "Закрытие чека";
		case sc_CancelReceipt: 			return "Аннулирование чека";
		case sc_SubTotal: 				return "Подытог чека";
		case sc_RepeatDoc: 				return "Печать копии чека (Повтор документа)";
		case sc_OpenReceipt: 			return "Открыть чек";
		case sc_CloseReceiptExt: 		return "Закрытие чека расширенное";
		case sc_ContinuePrint: 			return "Продолжение печати";
		case sc_LoadGraphics: 			return "Загрузка графики";
		case sc_PrintGraphics: 			return "Печать графики";
		case sc_PrintEAN13: 			return "Печать штрих-кода EAN-13";
		case sc_PrintGraphicsExt: 		return "Печать расширенной графики";
		case sc_LoadGraphicsExt: 		return "Загрузка расширенной графики";
		case sc_PrintLine: 				return "Печать графической линии";
		case sc_ZReportCash: 			return "Суточный отчет с гашением в буфер";
		case sc_PrintBarcodeHardware: 	return "Печать штрих-кода средствами принтера";
		case sc_GetPrinterStateExt: 	return "Запрос состояния принтера длинное";
		case sc_GetPrinterState: 		return "Запрос состояния принтера короткое";
		case sc_LoadData: 				return "Загрузка данных";
		case sc_PrintBarcode: 			return "Печать многомерного штрих-кода";
		case sc_OpenShift: 				return "Открыть смену";
		case sc_ExtRequest: 			return "Расширенный запрос";
		case sc_GetDeviceType: 			return "Получить тип устройства";
		case sc_GetFNStatus: 			return "Запрос статуса ФН";
		case sc_GetFNNumber: 			return "Запрос номера ФН";
		case sc_GetFNPeriod: 			return "Запрос срока действия ФН";
		case sc_GetFNVersion: 			return "Запрос версии ФН";
		case sc_BeginRegistrationRep: 	return "Начать отчет о регистрации ККТ";
		case sc_CloseRegistrationRep: 	return "Сформировать отчѐт о регистрации ККТ";
		case sc_ResetFN: 				return "Сброс состояния ФН";
		case sc_CancelDocFN: 			return "Отменить документ в ФН";
		case sc_GetLastFiscalizationTot:return "Запрос итогов последней фискализации";
		case sc_GetDocFromNum: 			return "Найти фискальный документ по номеру";
		case sc_OpenShiftFN: 			return "Открыть смену в ФН";
		case sc_TLVSend: 				return "Передать произвольную TLV структуру";
		case sc_DiscountOperation: 		return "Операция со скидками и надбавками";
		case sc_GetOpenFNParam: 		return "Запрос параметра открытия ФН";
		case sc_GetDataConsFromBuffer: 	return "Запросить о наличие данных в буфере";
		case sc_GetDataBlockFromBuffer: return "Прочитать блок данных из буфера";
		case sc_StartDataSaveToBuffer: 	return "Начать запись данных в буфер";
		case sc_DataSaveToBuffer: 		return "Записать блок данных в буфер";
		case sc_ReregistrationRep: 		return "Сформировать отчѐт о перерегистрации ККТ";
		case sc_OpenCorrectionReceipt: 	return "Начать формирование чека коррекции";
		case sc_CloseCorrectionReceipt: return "Сформировать чек коррекции FF36H";
		case sc_StartPaymentStatusRep: 	return "Начать формирование отчѐта о состоянии расчѐтов";
		case sc_PaymentStatusRep: 		return "Сформировать отчѐт о состоянии расчѐтов";
		case sc_GetStatusInfExchange: 	return "Получить статус информационного обмена";
		case sc_GetTLVDoc: 				return "Запросить фискальный документ в TLV формате";
		case sc_ReadTLVDoc: 			return "Чтение TLV фискального документа";
		case sc_GetOFDDocStatus: 		return "Запрос квитанции о получении данных в ОФД по номеру документа";
		case sc_StartClosingFiscalMode: return "Начать закрытие фискального режима";
		case sc_CloseFiscalMode: 		return "Закрыть фискальный режим";
		case sc_GetFDDocWithoutCheck: 	return "Запрос количества ФД на которые нет квитанции";
		case sc_GetCurrentShiftParam: 	return "Запрос параметров текущей смены";
		case sc_OpenFnShift: 			return "Начать открытие смены";
		case sc_StartCloseFnShift: 		return "Начать закрытие смены";
		case sc_CloseFnShift: 			return "Закрыть смену в ФН";
		case sc_CloseReceiptV2: 		return "Закрытие чека расширенное вариант №2";
		case sc_OperationV2: 			return "Операция V2";
		case sc_CreateCorrectionReceipt:return "Сформировать чек коррекции V2";
		case sc_DiscountRosneft: 		return "Скидка, надбавка на чек для Роснефти";
		case sc_GetFiscalizationTotV2: 	return "Запрос итогов фискализации (перерегистрации)V2";
		case sc_SendOperationTLV: 		return "Передать произвольную TLV структуру привязанную к операции";
		case sc_SaveFnToSD: 			return "Запись блока данных прошивки ФР на SD карту";
		case sc_OnlinePayment: 			return "Онлайн платѐж";
		case sc_OnlinePaymentStatus: 	return "Статус онлайн платѐжа";
		case sc_GetLastOnlinePayment: 	return "Получить реквизит последнего онлайн платѐжа";
		case sc_GetFiscalizationParam: 	return "Запрос параметра фискализации";
		case sc_CheckMarkGood: 			return "Проверка маркированного товара";
		case sc_SyncRegisters: 			return "Синхронизировать регистры со счѐтчиком ФН";
		case sc_GetMemorySize: 			return "Запрос ресурса свободной памяти в ФН";
		case sc_SendTLVFromBuffer: 		return "Передача в ФН TLV из буфера";
		case sc_GetRandomData: 			return "Получить случайную последовательность";
		case sc_Authorization: 			return "Авторизоваться";
		case sc_SetMarkGoodPosition: 	return "Привязка маркированного товара к позиции";
		case sc_GetMarkState: 			return "Получить состояние по передаче уведомлений о реализации маркированных товаров";
		case sc_ComplMarkGood: 			return "Принять или отвергнуть введенный код маркировки";

		default: 						return "Undefined";
	}
}

const gchar* error_to_str(ShtrihError error)
{
	switch(error)
	{
		case se_NoError: 					return "Ошибок нет";
		case se_CommandError: 				return "Неизвестная команда, неверный формат посылки или неизвестные параметры";
		case se_FNStateError: 				return "Другое состояние ФН";
		case se_FNError: 					return "Отказ ФН";
		case se_KSError: 					return "Отказ КС";
		case se_ParamFNError: 				return "Параметры команды не соответствуют сроку жизни ФН";
		case se_DateTimeFault: 				return "Неверные дата и/или время";
		case se_DataNotFound: 				return "Нет запрошенных данных";
		case se_ParamFault: 				return "Некорректное значение параметров команды";
		case se_TLVSizeError: 				return "Превышение размеров TLV данных";
		case se_TransportConnectionError: 	return "Нет транспортного соединения";
		case se_BadFN: 						return "Исчерпан ресурс ФН";
		case se_ResourceFN: 				return "Ограничение ресурса ФН";
		case se_24h: 						return "Продолжительность смены более 24 часов";
		case se_DateTimeError: 				return "Некорректные данные о промежутке времени между фискальными документами";
		case se_OFDMessageError: 			return "Сообщение от ОФД не может быть принято";
		case se_FNTimeout: 					return "Таймаут обмена с ФН";
		case se_FNNotResponse: 				return "ФН не отвечает";
		case se_FaultParam: 				return "Некорректные параметры в команде";
		case se_NoData:		 				return "Нет данных";
		case se_ParamSettingsCollision: 	return "Некорректный параметр при данных настройках";
		case se_ParamVersionCollision: 		return "Некорректные параметры в команде для данной реализации ККТ";
		case se_CommandVersionCollision: 	return "Команда не поддерживается в данной реализации ККТ";
		case se_EPROMError: 				return "Ошибка в ПЗУ";
		case se_SoftwareError: 				return "Внутренняя ошибка ПО ККТ";
		case se_ShiftOverload: 				return "Переполнение накопления по надбавкам в смене";
		case se_OperationOpenShift: 		return "Смена открыта операция невозможна";
		case se_OperationOpenShift2: 		return "Смена открыта операция невозможна";
		case se_ShiftSectionOverload: 		return "Переполнение накопления по секциям в смене";
		case se_ShiftDiscountOverload: 		return "Переполнение накопления по скидкам в смене";
		case se_DiscountRangeOverload: 		return "Переполнение диапазона скидок";
		case se_CashRangeOverload: 			return "Переполнение диапазона оплаты наличными";
		case se_Type2RangeOverload: 		return "Переполнение диапазона оплаты типом 2";
		case se_Type3RangeOverload: 		return "Переполнение диапазона оплаты типом 3";
		case se_Type4RangeOverload: 		return "Переполнение диапазона оплаты типом 4";
		case se_SumError: 					return "Cумма всех типов оплаты меньше итога чека";
		case se_CashError: 					return "Не хватает наличности в кассе";
		case se_TaxError: 					return "Переполнение накопления по налогам в смене";
		case se_OverloadTotalReceipt: 		return "Переполнение итога чека";
		case se_OperationErrorOpenReceipt: 	return "Операция невозможна в открытом чеке данного типа";
		case se_OpenReceiptError: 			return "Открыт чек – операция невозможна";
		case se_ReceiptBufferError: 		return "Буфер чека переполнен";
		case se_TaxShiftError: 				return "Переполнение накопления по обороту налогов в смене";
		case se_BnSumError: 				return "Вносимая безналичной оплатой сумма больше суммы чека";
		case se_Shift24: 					return "Смена превысила 24 часа";
		case se_PasswordError: 				return "Неверный пароль";
		case se_PrintInProgress: 			return "Идет печать результатов выполнения предыдущей команды";
		case se_CashShiftError: 			return "Переполнение накоплений наличными в смене";
		case se_Type2ShiftError: 			return "Переполнение накоплений по типу оплаты 2 в смене";
		case se_Type3ShiftError: 			return "Переполнение накоплений по типу оплаты 3 в смене";
		case se_Type4ShiftError: 			return "Переполнение накоплений по типу оплаты 4 в смене";
		case se_CloseReceiptError: 			return "Чек закрыт – операция невозможна";
		case se_RepeatDocError: 			return "Нет документа для повтора";
		case se_WaitContinuePrintCmd: 		return "Ожидание команды продолжения печати";
		case se_OperatorDocError: 			return "Документ открыт другим оператором";
		case se_OverloadRangeBonus: 		return "Переполнение диапазона надбавок";
		case se_LowVoltage: 				return "Понижено напряжение 24В";
		case se_TableNotDefine: 			return "Таблица не определена";
		case se_OperationError: 			return "Неверная операция";
		case se_NegativTotal: 				return "Отрицательный итог чека";
		case se_OverloadMultiply: 			return "Переполнение при умножении";
		case se_OverloadPriceRange: 		return "Переполнение диапазона цены";
		case se_OverloadCountRange: 		return "Переполнение диапазона количества";
		case se_OverloadDepRange: 			return "Переполнение диапазона отдела";
		case se_NoMoneyInSection: 			return "Не хватает денег в секции";
		case se_OverloadSectionMoney: 		return "Переполнение денег в секции";
		case se_NoTaxMoney: 				return "Не хватает денег по обороту налогов";
		case se_OverloadTaxMoney: 			return "Переполнение денег по обороту налогов";
		case se_PowerErrorI2C: 				return "Ошибка питания в момент ответа по I2C";
		case se_NoPaper: 					return "Нет чековой ленты";
		case se_NoSingleTaxMoney: 			return "Не хватает денег по налогу";
		case se_OverloadSingleTaxMoney: 	return "Переполнение денег по налогу";
		case se_OverloadOutputShift: 		return "Переполнение по выплате в смене";
		case se_CutterError: 				return "Ошибка отрезчика";
		case se_CommandNotSuppInSubMode: 	return "Команда не поддерживается в данном подрежиме";
		case se_CommandNotSuppInMode: 		return "Команда не поддерживается в данном режиме";
		case se_RAMError: 					return "Ошибка ОЗУ";
		case se_PowerError: 				return "Ошибка питания";
		case se_PrinterSensorError: 		return "Ошибка принтера: нет сигнала с датчиков";
		case se_SofwareUpload: 				return "Замена ПО";
		case se_FieldReadOnly: 				return "Поле не редактируется";
		case se_HardwareError: 				return "Ошибка оборудования";
		case se_DateError: 					return "Не совпадает дата";
		case se_FormatDateError: 			return "Неверный формат даты";
		case se_LengthError: 				return "Неверное значение в поле длины";
		case se_OverloadTotalRangeError: 	return "Переполнение диапазона итога чека";
		case se_OverloadCash: 				return "Переполнение наличности";
		case se_OverloadSaleShift: 			return "Переполнение по продажам в смене";
		case se_OverloadBuyShift: 			return "Переполнение по покупкам в смене";
		case se_OverloadReturnSaleShift: 	return "Переполнение по возвратам продаж в смене";
		case se_OverloadReturnBuyShift: 	return "Переполнение по возвратам покупок в смене";
		case se_OverloadIncomeShift: 		return "Переполнение по внесению в смене";
		case se_OverloadBonusShift: 		return "Переполнение по надбавкам в чеке";
		case se_OverloadDiscountShift: 		return "Переполнение по скидкам в чеке";
		case se_NegativeBonusTotal: 		return "Отрицательный итог надбавки в чеке";
		case se_NegativeDiscountTotal: 		return "Отрицательный итог скидки в чеке";
		case se_ZeroTotal: 					return "Нулевой итог чека";
		case se_VeryBigFieldSize: 			return "Поле превышает размер, установленный в настройках";
		case se_overloadPrintArrea: 		return "Выход за границу поля печати при данных настройках шрифта";
		case se_FieldsCollision: 			return "Наложение полей";
		case se_RAMRestore: 				return "Восстановление ОЗУ прошло успешно";
		case se_OverloadLimitOperation: 	return "Исчерпан лимит операций в чеке";
		case se_MTError: 					return "Запрещена работа с маркированным товарами";
		case se_FaultSequence: 				return "Неверная последовательность команд группы BXh";
		case se_MTBlocked: 					return "Работа с маркированными товарами временно заблокирована";
		case se_OverloadMTTable: 			return "Переполнена таблица проверки кодов маркировки";
		case se_BlockTLVError: 				return "В блоке TLV отсутствуют необходимые реквизиты";
		case se_2007Error: 					return "В реквизите 2007 содержится КМ, который ранее не проверялся в ФН";
		case se_DateTimeControl: 			return "Контроль даты и времени (подтвердите дату и время)";
		case se_HiVoltage: 					return "Превышение напряжения в блоке питания";
		case se_ShiftNumError: 				return "Несовпадение номеров смен";
		case se_FieldReadOnlyInMode: 		return "Поле не редактируется в данном режиме";
		case se_PrinterConnectError: 		return "Нет связи с принтером или отсутствуют импульсы от таходатчика";

		default: 							return "Undefined";
	}
}

const gchar* shtrih_mode_to_str(ShtrihMode mode)
{
	switch(mode)
	{
		case sm_DataSending: 				return "Выдача данных";
		case sm_ShiftOpened: 				return "Открытая смена, 24 часа не кончились";
		case sm_ShiftOpened24: 				return "Открытая смена, 24 часа кончились";
		case sm_ShiftClosed: 				return "Закрытая смена";
		case sm_Blocked: 					return "Блокировка по неправильному паролю налогового инспектора";
		case sm_DateConfirmation: 			return "Ожидание подтверждения ввода даты";
		case sm_DeciamlPointSetup: 			return "Разрешение изменения положения десятичной точки";
		case sm_DocOpened: 					return "Открытый документ";
		case sm_TechReset: 					return "Режим разрешения технологического обнуления";
		case sm_TestPrint: 					return "Тестовый прогон";
		case sm_FiscalRep: 					return "Печать полного фискального отчета";
		case sm_FiscalA4: 					return "Работа с фискальным подкладным документом";
		case sm_PrintA4: 					return "Печать подкладного документа";
		case sm_FiscalA4Complete: 			return "Фискальный подкладной документ сформирован";

		default: 							return "Undefined";
	}
}

const gchar* shtrih_doc_opened_status_to_str(ShtrihDocOpenedStatus status)
{
	switch(status)
	{
		case sdos_Sale: 					return "Продажа";
		case sdos_Buy: 						return "Покупка";
		case sdos_ReturnSale: 				return "Возврат продажи";
		case sdos_ReturnBuy: 				return "Возврат покупки";
		case sdos_NotFiscal: 				return "Нефискальный";

		default: 							return "Undefined";
	}
}

const gchar* shtrih_fiscal_a4_status_to_str(ShtrihFiscalA4Status status)
{
	switch(status)
	{
		case sfas_Sale: 					return "Продажа";
		case sfas_Buy: 						return "Покупка";
		case sfas_ReturnSale: 				return "Возврат продажи";
		case sfas_ReturnBuy: 				return "Возврат покупки";

		default: 							return "Undefined";
	}
}

const gchar* shtrih_print_a4_status_to_str(ShtrihPrintA4Status status)
{
	switch(status)
	{
		case spas_WaitLoad: 				return "Ожидание загрузки";
		case spas_Loading: 					return "Загрузка и позиционирование";
		case spas_Positioning: 				return "Позиционирование";
		case spas_Printing: 				return "Печать";
		case spas_Printed: 					return "Печать закончена";
		case spas_Presenting: 				return "Выброс документа";
		case spas_WaitRemoving: 			return "Ожидание извлечения";

		default: 							return "Undefined";
	}
}

const gchar* shtrih_submode_to_str(ShtrihSubMode submode)
{
	switch(submode)
	{
		case ssm_Idle: 						return "Бумага есть";
		case ssm_NoPaperPassive: 			return "Пассивное отсутствие бумаги";
		case ssm_NoPaperActive: 			return "Активное отсутствие бумаги";
		case ssm_WaitingPrintContinue: 		return "ККТ ждет команду продолжения печати";
		case ssm_FiscalRep: 				return "Фаза печати операции полных фискальных отчетов";
		case ssm_Printing: 					return "Фаза печати операции";

		default: 							return "Undefined";
	}
}
