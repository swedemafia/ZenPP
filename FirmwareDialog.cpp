#include "Precomp.h"

FirmwareDialog::FirmwareDialog(CONST FirmwareManager::FirmwareModificationPurpose& Purpose) :
	m_Purpose(Purpose)
{

}

HWND FirmwareDialog::GetDescriptorEditHandle(VOID) CONST
{
	return m_EditDescriptors;
}

INT_PTR FirmwareDialog::HandleMessage(CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam)
{
	FirmwareDialog* Dialog = reinterpret_cast<FirmwareDialog*>(GetWindowLongPtr(m_hWnd, GWLP_USERDATA));

	switch (Message)
	{
	case WM_CLOSE:					return Dialog->OnClose();
	case WM_COMMAND:				return Dialog->OnCommand(wParam, lParam);
	case WM_DESTROY:				return Dialog->OnDestroy();
	case WM_DEVICECHANGE:			return Dialog->OnDeviceChange(wParam);
	case WM_INITDIALOG:				return Dialog->OnInitDialog();
	}

	return FALSE;
}

INT_PTR FirmwareDialog::OnCommandButtonFirmwareBack(VOID)
{
	ChangeModificationStep(FALSE);
	return TRUE;
}

INT_PTR FirmwareDialog::OnCommandButtonFirmwareNext(VOID)
{
	ChangeModificationStep(TRUE);
	return TRUE;
}

INT_PTR FirmwareDialog::OnClose(VOID)
{
	if(m_Manager && !m_Manager->IsModifying())
		DestroyWindow(m_hWnd);

	return TRUE;
}

INT_PTR FirmwareDialog::OnCommand(CONST WPARAM wParam, CONST LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case BUTTON_FIRMWARE_BACK:							return OnCommandButtonFirmwareBack();
	case BUTTON_FIRMWARE_CANCEL:						return OnClose();
	case BUTTON_FIRMWARE_NEXT:							return OnCommandButtonFirmwareNext();
	}
	return FALSE;
}

INT_PTR FirmwareDialog::OnDestroy(VOID)
{
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG)NULL);
	App->DestroyFirmwareDialog();
	return TRUE;
}

INT_PTR FirmwareDialog::OnDeviceChange(CONST WPARAM wParam)
{
	// When a device arrives or is removed
	if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE)
	{
		// Only update for our device only
		if((m_Manager->IsDeviceFound() != m_Manager->SearchForDevice()) && !m_Manager->UpdateCompleted())
			UpdateCapabilities();
	}

	return TRUE;
}

INT_PTR FirmwareDialog::OnInitDialog(VOID)
{
	try
	{
		// Get dialog control handles
		m_ButtonBack = GetDlgItem(m_hWnd, BUTTON_FIRMWARE_BACK);
		m_ButtonCancel = GetDlgItem(m_hWnd, BUTTON_FIRMWARE_CANCEL);
		m_ButtonNext = GetDlgItem(m_hWnd, BUTTON_FIRMWARE_NEXT);
		m_EditDescriptors = GetDlgItem(m_hWnd, EDIT_FIRMWARE_DESCRIPTORS);
		m_LabelInstructions = GetDlgItem(m_hWnd, LABEL_FIRMWARE_INSTRUCTIONS);
		m_LabelStatus = GetDlgItem(m_hWnd, LABEL_FIRMWARE_STATUS);
		m_LabelStep = GetDlgItem(m_hWnd, LABEL_FIRMWARE_STEP);
		m_PictureControl = GetDlgItem(m_hWnd, PICTURE_FIRMWARE);
		m_ProgressBar = GetDlgItem(m_hWnd, PROGRESS_FIRMWARE);

		// Validate we captured the control handles
		if (!m_ButtonBack || !m_ButtonCancel || !m_ButtonNext || !m_EditDescriptors || !m_LabelInstructions || !m_LabelStatus || !m_LabelStep || !m_PictureControl || !m_ProgressBar)
			throw std::wstring(L"An error occured while initializing the Firmware Manager dialog.");

		// Create Firmware Manager object
		m_Manager = std::make_unique<FirmwareManager>(m_Purpose);

		// Register device change notifications;
		RegisterDeviceNotifications(m_Manager->GetGUID());

		// Set Progress Bar parameters
		SendMessage(m_ProgressBar, PBM_SETBARCOLOR, 0, static_cast<LPARAM>(TEAL));
		SendMessage(m_ProgressBar, PBM_SETPOS, 0, 0);

		// Update window caption
		SetWindowText(m_hWnd, std::wstring_view(L"Zen++ Firmware Manager - " + m_WindowTitle[m_Purpose]).data());

	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
		OnClose();
	}
	catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Unable to create the FirmwareManager object; insufficient memory is available to complete the required operation.");
		OnClose();
	}

	return FALSE;
}

VOID FirmwareDialog::ChangeModificationStep(_In_ CONST BOOLEAN Forwards)
{
	m_Step = Forwards ? ++m_Step : --m_Step;
	UpdateCapabilities();
}

VOID FirmwareDialog::UpdateCapabilities(VOID)
{
	if (m_Step < 2)
	{
		if (!m_Step) {
			EnableWindow(m_ButtonBack, FALSE);
			EnableWindow(m_ButtonNext, TRUE);
			SetWindowText(m_LabelStatus, L"The CONSOLE/PC USB port is located on the backside of your Cronus Zen.");
			UpdateInstructions();
		}
		else {
			BOOL DeviceFound = m_Manager->IsDeviceFound();
			EnableWindow(m_ButtonBack, !DeviceFound);
			EnableWindow(m_ButtonNext, DeviceFound);
			SetWindowText(m_LabelStatus, m_DeviceState[DeviceFound].c_str());
			UpdateInstructions();
		}
	}
	else {
		if (m_Manager->IsDeviceFound()) {
			EnableWindow(m_ButtonNext, FALSE);
			ShowWindow(m_EditDescriptors, SW_SHOW);
			ShowWindow(m_PictureControl, SW_HIDE);
			ShowWindow(m_ProgressBar, SW_SHOW);
			
			// Perform firmware modification
			if (!m_Manager->PerformModification())
				OnClose();

		}
		else {
			// TODO: finish this
			OnClose();
		}
	}
}

VOID FirmwareDialog::UpdateInstructions(VOID)
{
		// Update instructions
		SetWindowText(m_LabelInstructions, m_Instructions[m_Step].c_str());
		SetWindowText(m_LabelStep, std::wstring_view(L"Step " + std::to_wstring(m_Step + 1) + L":").data());

		// Only update Bitmap for the first two steps
		if (m_Step != 2)
		{
			try
			{
				// Update Picture control
				HBITMAP Bitmap = LoadBitmap(App->GetInstance(), MAKEINTRESOURCE(BITMAP_FIRMWARE_STEP1 + m_Step));

				// Validate Bitmap was loaded
				if (!Bitmap)
					throw std::wstring(L"An error occured while loading the next bitmap.");

				// Update the Picture control
				SendMessage(m_PictureControl, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(Bitmap));

				// Delete Bitmap
				DeleteObject(Bitmap);
			}
			catch (CONST std::wstring& CustomMesage)
			{
				App->DisplayError(CustomMesage);
			}
		}
}

VOID FirmwareDialog::UpdateProgressBar(_In_ CONST DWORD Value, _In_ CONST DWORD Maximum)
{
	UINT Percentage = (UINT)(((DOUBLE)Value / Maximum) * 100);

	// Determine status
	if (Value != Maximum)
		SetWindowText(m_LabelStatus, std::wstring(std::to_wstring(Percentage) + L"% downloaded to device (" + std::to_wstring(Value) + L" of " + std::to_wstring(Maximum) + L" bytes processed).").c_str());
	else if (m_Purpose == FirmwareManager::EraseFirmware)
		SetWindowText(m_LabelStatus, L"Successfully erased the firmware on your device!");
	else
		SetWindowText(m_LabelStatus, L"Successfully flashed firmware to your device!");

	// Update Progress control
	SendMessage(m_ProgressBar, PBM_SETPOS, static_cast<WPARAM>(Percentage), 0);
}