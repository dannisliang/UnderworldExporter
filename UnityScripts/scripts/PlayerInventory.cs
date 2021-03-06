﻿using UnityEngine;
using System.Collections;

public class PlayerInventory : MonoBehaviour {

	//The game objects at the various slots. (not in use?)
	//private GameObject Helm;
	//private GameObject Chest;
	//private GameObject Legs;
	//private GameObject Boots;
	//private GameObject Gloves;
	//private GameObject LeftHand;
	//private GameObject RightHand;
	//private GameObject LeftRing;
	//private GameObject RightRing;
	//private GameObject LeftShoulder;
	//private GameObject RightShoulder;
	//private GameObject[] BackPack= new GameObject[8];

	public string ObjectInHand; //What is the current active object held by the player
	public bool JustPickedup; //Has the player just picked something up.
	//The game object name of the item.
	public string sHelm;
	public string sChest;
	public string sLegs;
	public string sBoots;
	public string sGloves;
	public string sLeftHand;
	public string sRightHand;
	public string sLeftRing;
	public string sRightRing;
	public string sLeftShoulder;
	public string sRightShoulder;
	public string[] sBackPack= new string[8];
	//public bool[] runes = new bool[24];

	//Force Redraw on next update.
	public bool bHelm=true;
	public bool bChest=true;
	public bool bLegs=true;
	public bool bBoots=true;
	public bool bGloves=true;
	public bool bLeftHand=true;
	public bool bRightHand=true;
	public bool bLeftRing=true;
	public bool bRightRing=true;
	public bool bLeftShoulder=true;
	public bool bRightShoulder=true;
	public bool[] bBackPack= new bool[8];

	//Links to the slots where the object will be displayed
	public UISprite Helm_f_Slot;
	public UISprite Chest_f_Slot;
	public UISprite Legs_f_Slot;
	public UISprite Boots_f_Slot;
	public UISprite Gloves_f_Slot;
	public UISprite Helm_m_Slot;
	public UISprite Chest_m_Slot;
	public UISprite Legs_m_Slot;
	public UISprite Boots_m_Slot;
	public UISprite Gloves_m_Slot;
	public UISprite LeftHand_Slot;
	public UISprite RightHand_Slot;
	public UISprite LeftRing_Slot;
	public UISprite RightRing_Slot;
	public UISprite LeftShoulder_Slot;
	public UISprite RightShoulder_Slot;
	public UISprite[] BackPack_Slot=new UISprite[8];

	public bool atTopLevel;
	public string currentContainer;

	private UWCharacter playerUW;

	private Container playerContainer;
	// Use this for initialization
	void Start () {
		atTopLevel=true;
		playerUW=GameObject.Find ("Gronk").GetComponent<UWCharacter>();
		playerContainer = GameObject.Find ("Gronk").GetComponent<Container>();
		for (int i =0;i<8;i++)
		{
			bBackPack[i]=true;
		}
	}
	
	// Update is called once per frame
	void Update () {
		if (playerUW.isFemale==true)
		{//female
			DisplayGameObject (sHelm,Helm_f_Slot,true,ref bHelm);
			DisplayGameObject (sChest,Chest_f_Slot,true,ref bChest);
			DisplayGameObject (sLegs,Legs_f_Slot,true,ref bLegs);
			DisplayGameObject (sBoots,Boots_f_Slot,true,ref bBoots);
			DisplayGameObject (sGloves,Gloves_f_Slot,true,ref bGloves);
		}
		else
		{//male
			DisplayGameObject (sHelm,Helm_m_Slot,true,ref bHelm);
			DisplayGameObject (sChest,Chest_m_Slot,true,ref bChest);
			DisplayGameObject (sLegs,Legs_m_Slot,true,ref bLegs);
			DisplayGameObject (sBoots,Boots_m_Slot,true,ref bBoots);
			DisplayGameObject (sGloves,Gloves_m_Slot,true,ref bGloves);
		}

		DisplayGameObject(sLeftShoulder,LeftShoulder_Slot,false,ref bLeftShoulder);
		DisplayGameObject(sRightShoulder,RightShoulder_Slot,false,ref bRightShoulder);
		DisplayGameObject(sLeftRing,LeftRing_Slot,false,ref bLeftRing);
		DisplayGameObject(sRightRing,RightRing_Slot,false,ref bRightRing);

		DisplayGameObject(sLeftHand,LeftHand_Slot,false,ref bLeftHand);
		DisplayGameObject(sRightHand,RightHand_Slot,false,ref bRightHand);

		for (int i = 0 ; i < 8; i++)
		{
			DisplayGameObject (sBackPack[i],BackPack_Slot[i],false,ref bBackPack[i]);
		}
	}

	void DisplayGameObject(string objName, UISprite Label, bool isEquipped, ref bool hasChanged)
	{
		if (hasChanged==true)
		{
			if (objName =="")
			{
				Label.spriteName="object_blank";
				hasChanged=false;
				return;
			}
			GameObject objToDisplay=GameObject.Find (objName);
			hasChanged=false;
			if (objToDisplay != null)
			{
				if (isEquipped==true)
				{
					Label.spriteName= objToDisplay.GetComponent<ObjectInteraction>().InventoryIconEquipString;
				}
				else
				{
					Debug.Log ("Displaying " + objToDisplay.GetComponent<ObjectInteraction>().InventoryString);
					Label.spriteName= objToDisplay.GetComponent<ObjectInteraction>().InventoryString;
				}
			}
			else
			{
				Debug.Log ("Displaying blank");
				Label.spriteName="object_blank";
			}
		}

	}

	public string ObjectPickedUp(int slotIndex, string sObjectInHand)
	{//Returns the game object of the object already in the slot
		string ExistingObject="";
		//Debug.Log ("looking for object " + sObjectInHand);
		//GameObject ObjectInHand = GameObject.Find (sObjectInHand);
		//if(ObjectInHand!=null)
		//{Debug.Log (ObjectInHand.name);
			//Debug.Log ("Object found");
			switch (slotIndex)
			{
		case 0://Helm
			if (InteractTwoObjects (sObjectInHand,sHelm,slotIndex) == false)
			{
				bHelm=true;
				ExistingObject=sHelm;
				sHelm=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 1://Chest
			if (InteractTwoObjects (sObjectInHand,sChest,slotIndex) == false)
			{
				bChest=true;
				ExistingObject=sChest;
				sChest=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 2://Leggings
			if (InteractTwoObjects (sObjectInHand,sLegs,slotIndex) == false)
			{
				bLegs=true;
				ExistingObject=sLegs;
				sLegs=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 3://Boots
			if (InteractTwoObjects (sObjectInHand,sBoots,slotIndex) == false)
			{
				bBoots=true;
				ExistingObject=sBoots;
				sBoots=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 4://Gloves
			if (InteractTwoObjects (sObjectInHand,sGloves,slotIndex) == false)
			{
				bGloves=true;
				ExistingObject=sGloves;
				sGloves=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 5://ShoulderRight
			if (InteractTwoObjects (sObjectInHand,sRightShoulder,slotIndex) == false)
			{
				bRightShoulder=true;
				ExistingObject=sRightShoulder;
				sRightShoulder=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 6://ShoulderLeft
			if (InteractTwoObjects (sObjectInHand,sLeftShoulder,slotIndex) == false)
			{
				bLeftShoulder=true;
				ExistingObject=sLeftShoulder;
				sLeftShoulder=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 7://HandRight
			if (InteractTwoObjects (sObjectInHand,sRightHand,slotIndex) == false)
			{
				bRightHand=true;
				ExistingObject=sRightHand;
				sRightHand=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 8://HandLeft
			if (InteractTwoObjects (sObjectInHand,sLeftHand,slotIndex) == false)
			{
				bLeftHand=true;
				ExistingObject=sLeftHand;
				sLeftHand=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 9://RingRight
			if (InteractTwoObjects (sObjectInHand,sRightRing,slotIndex) == false)
			{
				bRightRing=true;
				ExistingObject=sRightRing;
				sRightRing=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
		case 10://RingLeft
			if (InteractTwoObjects (sObjectInHand,sLeftRing,slotIndex) == false)
			{
				bLeftRing=true;
				ExistingObject=sLeftRing;
				sLeftRing=sObjectInHand;
			}
			else
			{
				ExistingObject="";
			}
			break;
			default://Inventory Slots 0-7		
				if ((slotIndex>=11)&&(slotIndex<=18))
				{
				if (InteractTwoObjects (sObjectInHand,sBackPack[slotIndex-11],slotIndex) == false)
				{
					bBackPack[slotIndex-11]=true;
					ExistingObject=sBackPack[slotIndex-11];
					sBackPack[slotIndex-11]=sObjectInHand;
					playerContainer.RemoveItemFromContainer (slotIndex-11);
					playerContainer.AddItemToContainer (sObjectInHand,slotIndex-11);
				}

				else
				{
					ExistingObject="";
				}

					
				}
				break;
			}
	//	}

		return ExistingObject;

	}

	public string GetObjectDescAtSlot(int SlotIndex)
	{
		string objectName=GetObjectAtSlot (SlotIndex);
		if (objectName!="")
		{
			ObjectInteraction objInt = GameObject.Find (objectName).GetComponent<ObjectInteraction>();
			if (objInt != null)
			{
				return objInt.LookDescription();
			}
			else
			{
				return "DESC NOT FOUND!!";
			}
		}
		else
		{
			return "DESC NOT FOUND!!";
		}
	}

	public string GetObjectAtSlot(int slotIndex)
	{
		switch (slotIndex)
			{
			case 0://Helm
				return sHelm;
			case 1://Chest
				return sChest;
			case 2://Leggings
				return sLegs;
			case 3://Boots
				return sBoots;
			case 4://Gloves
				return sGloves;
			case 5://ShoulderRight
				return sRightShoulder;
			case 6://ShoulderLeft
				return sLeftShoulder;
			case 7://HandRight
				return sRightHand;
			case 8://HandLeft
				return sLeftHand;
			case 9://RingRight
				return sRightRing;
			case 10://RingLeft
				return sLeftRing;
			default://Inventory Slots 0-7		
				if ((slotIndex>=11)&&(slotIndex<=18))
					{
						return sBackPack[slotIndex-11];
					}
					else
					{
						return "";
					}
		}
	}

	public void SetObjectAtSlot(int slotIndex,string sObject)
	{
		switch (slotIndex)
		{
		case 0://Helm
			 sHelm=sObject;
	         bHelm=true;
			break;
		case 1://Chest
			sChest=sObject;
			bChest=true;
			break;
		case 2://Leggings
			sLegs=sObject;
			bLegs=true;
			break;
		case 3://Boots
			sBoots=sObject;
			bBoots=true;
			break;
		case 4://Gloves
			sGloves=sObject;
			bGloves=true;
			break;
		case 5://ShoulderRight
			sRightShoulder=sObject;
			bRightShoulder=true;
			break;
		case 6://ShoulderLeft
			sLeftShoulder=sObject;
			bLeftShoulder =true;
			break;
		case 7://HandRight
			sRightHand=sObject;
			bRightHand =true;
			break;
		case 8://HandLeft
			sLeftHand=sObject;
			bLeftHand=true;
			break;
		case 9://RingRight
			sRightRing=sObject;
			bRightRing=true;
			break;
		case 10://RingLeft
			sLeftRing=sObject;
			bLeftRing=true;
			break;
		default://Inventory Slots 0-7		
			if ((slotIndex>=11)&&(slotIndex<=18))
			{
				sBackPack[slotIndex-11]=sObject;
				bBackPack[slotIndex-11]=true;
			}
			break;
		}
	}

	public bool InteractTwoObjects(string sObjectInHand, string sObjectUsedOn,int slotIndex)
	{//How two objects affect each other. eventually I will implement a table for object combining.
		Debug.Log ("Interacting " + sObjectInHand + " and " + sObjectUsedOn);
		//returns true if they have an effect on each other.
		if ((sObjectInHand !="") && (sObjectUsedOn !=""))
		{//Object is being used on something.
			GameObject objInHand= GameObject.Find (sObjectInHand);
			GameObject objUseOn = GameObject.Find (sObjectUsedOn);
			if(objUseOn.GetComponent<ObjectInteraction>() ==null)
			{//Object has no interaction component.
				ObjectInHand="";
				playerUW.CursorIcon= playerUW.CursorIconDefault;
				playerUW.CurrObjectSprite = "";
				return false;
			}
			//Add item to container
			if (objUseOn.GetComponent<ObjectInteraction>().isContainer)
			{
				if(Container.AddObjectToContainer(objInHand,objUseOn))
				{
					ObjectInHand="";
					playerUW.CursorIcon= playerUW.CursorIconDefault;
					playerUW.CurrObjectSprite = "";
					//ObjectInHand="";
				}
			}
			
			if(objUseOn.GetComponent<ObjectInteraction>().isRuneBag)
			{//Add a runestone to the rune bag.
				if(objInHand.GetComponent<ObjectInteraction>().isRuneStone)
				{
					UWCharacter playerUW = GameObject.Find ("Gronk").GetComponent<UWCharacter>();
					playerUW.Runes[objInHand.GetComponent<ObjectInteraction>().item_id-232]=true;
					//Add rune to rune bag.
					GameObject.Destroy(objInHand);
					ObjectInHand="";
					playerUW.CursorIcon= playerUW.CursorIconDefault;
					playerUW.CurrObjectSprite = "";
				}
				return true;
			}

			if ((objUseOn.GetComponent<ObjectInteraction>().isDoor) && (objInHand.GetComponent<ObjectInteraction>().isKey) )
				{
				//Try and use a key on a door.
				Debug.Log ("A key is used on a door");
				if(objUseOn.GetComponent<ObjectInteraction>().Link==objInHand.GetComponent<ObjectInteraction>().Owner)
					{
					Debug.Log ("A key is used to unlock a door");
					DoorControl DC=objUseOn.GetComponent<DoorControl>();
					DC.ToggleLock();
					}
				ObjectInHand="";
				playerUW.CursorIcon= playerUW.CursorIconDefault;
				playerUW.CurrObjectSprite = "";				
				}
			return true;
		}
		else
		{//Object is just being placed in a slot. 
			if (ObjectInHand!="")
			{
				GameObject objInHand= GameObject.Find (sObjectInHand);
				//Container subContainer = objUseOn.GetComponent<Container>();
				if (objInHand.GetComponent<ObjectInteraction>().isContainer)
				{
					//PlayerInventory pInv = GameObject.Find ("Gronk").GetComponent<PlayerInventory>();
					Container subContainer=objInHand.GetComponent<Container>();
					if (slotIndex >=11)
					{//Object is being added to a bag container
						subContainer.ContainerParent=currentContainer;
					}
					else
					{//object is being added to an equipment slot
						subContainer.ContainerParent="Gronk";
					}
					
				}
			}
			return false;
		}
		
	}


}
