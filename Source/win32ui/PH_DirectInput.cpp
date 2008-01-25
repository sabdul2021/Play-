#include "PH_DirectInput.h"

#define DIBUFFERSIZE	(10)

using namespace std::tr1;

CPH_DirectInput::CPH_DirectInput(HWND hWnd)
{
	m_hWnd = hWnd;
	m_pDI = NULL;
	m_pKeyboard = NULL;

	Initialize();
}

CPH_DirectInput::~CPH_DirectInput()
{
	if(m_pKeyboard != NULL)
	{
		m_pKeyboard->Release();
	}
	if(m_pDI != NULL)
	{
		m_pDI->Release();
	}
}

CPadHandler::FactoryFunction CPH_DirectInput::GetFactoryFunction(HWND hWnd)
{
    return bind(&CPH_DirectInput::PadHandlerFactory, hWnd);
}

CPadHandler* CPH_DirectInput::PadHandlerFactory(HWND hWnd)
{
	return new CPH_DirectInput(hWnd);
}

void CPH_DirectInput::Initialize()
{
	DIPROPDWORD p;

	DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, NULL);

	m_pDI->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL);
	m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	m_pKeyboard->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	memset(&p, 0, sizeof(DIPROPDWORD));
	p.diph.dwSize		= sizeof(DIPROPDWORD);
	p.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	p.diph.dwHow		= DIPH_DEVICE;
	p.diph.dwObj		= 0;
	p.dwData			= DIBUFFERSIZE;

	m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &p.diph);

	m_pKeyboard->Acquire();
}

void CPH_DirectInput::Update(uint8* ram)
{
	DWORD nElements, i;
	HRESULT hRet;
	CPadListener::BUTTON nButton;
	DIDEVICEOBJECTDATA d[DIBUFFERSIZE];

	nElements = DIBUFFERSIZE;
	hRet = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), d, &nElements, 0);
	if(FAILED(hRet))
	{
		m_pKeyboard->Acquire();
		return;
	}

	for(i = 0; i < nElements; i++)
	{
		if(TranslateKey(d[i].dwOfs, &nButton))
		{
            for(ListenerList::iterator listenerIterator(m_listeners.begin()); 
                listenerIterator != m_listeners.end(); listenerIterator++)
			{
	            CPadListener* pListener(*listenerIterator);
				pListener->SetButtonState(0, nButton, (d[i].dwData & 0x80) ? true : false, ram);
			}
		}
	}	
}

bool CPH_DirectInput::TranslateKey(uint32 nSrc, CPadListener::BUTTON* nDst)
{
	switch(nSrc)
	{
	case DIK_RETURN:
		(*nDst) = CPadListener::BUTTON_START;
		return true;
		break;
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		(*nDst) = CPadListener::BUTTON_SELECT;
		return true;
		break;
	case DIK_LEFT:
		(*nDst) = CPadListener::BUTTON_LEFT;
		return true;
		break;
	case DIK_UP:
		(*nDst) = CPadListener::BUTTON_UP;
		return true;
		break;
	case DIK_DOWN:
		(*nDst) = CPadListener::BUTTON_DOWN;
		return true;
		break;
	case DIK_RIGHT:
		(*nDst) = CPadListener::BUTTON_RIGHT;
		return true;
		break;
	case DIK_A:
		(*nDst) = CPadListener::BUTTON_SQUARE;
		return true;
		break;
	case DIK_Z:
		(*nDst) = CPadListener::BUTTON_CROSS;
		return true;
		break;
	case DIK_S:
		(*nDst) = CPadListener::BUTTON_TRIANGLE;
		return true;
		break;
	case DIK_X:
		(*nDst) = CPadListener::BUTTON_CIRCLE;
		return true;
		break;
	default:
		return false;
		break;
	}
}
